/*  MapImport
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#define TIXML_USE_TICPP

#include <tpserver/logging.h>

//XML related function
#include <tpserver/tinyxml/tinyxml.h>
#include "mapimport.h"

//Map creation
#include <tpserver/object.h>
#include <tpserver/game.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>

#include <planet.h>
#include <constellation.h>
#include <staticobject.h>
#include <graph.h>
#include <risk.h>
#include <map.h>
#include <tpserver/rulesetsupport/nameselector.h>

#include <sstream>
#include <map>
#include <string>

//Tyler's "hacky define :p"
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__PRETTY_FUNCTION__))

namespace RiskRuleset {

using std::string;
using std::map;

typedef std::map<string,IGObject::Ptr > StrToObjMap;

#define ASIZEOF(x) (sizeof(x)/sizeof(x[0]))

static char const * const defaultConstellationMedia[] = {
    "nebula1", "nebula2", "nebula3", "nebula4", "nebula5"
};

static char const * const defaultSystemMedia[] = {
  "blue", "red", "yellow", "purple-large", 
  "purple-small", "rainbow"
};

static char const * const defaultPlanetMedia[] = {
  "barren1", "barren2", "barren3", "desert1", "desert2",
  "desert3", "gasgiant1", "gasgiant2", "gasgiant3",
  "inferno1", "inferno2", "inferno3", "ocean1", "ocean2",
  "ocean3", "radiated1", "radiated2", "radiaged3",
  "swamp1", "swamp2", "swamp3", "terran1", "terran2",
  "terran3", "toxic1", "toxic2", "toxic3", "tundra1",
  "tundra2", "tundra3"
};

MapImport::MapImport(){
    constellationmedia = new NamesSet(Game::getGame()->getRandom(), defaultConstellationMedia, ASIZEOF(defaultConstellationMedia), true, "");
    systemmedia = new NamesSet(Game::getGame()->getRandom(), defaultSystemMedia, ASIZEOF(defaultSystemMedia), true, "");
    planetmedia = new NamesSet(Game::getGame()->getRandom(), defaultPlanetMedia, ASIZEOF(defaultPlanetMedia), true, "");
}

bool MapImport::importMapFromFile(string filename, IGObject& universe){
   bool loadedMapOkay = false;
   Logger::getLogger()->debug("Trying to load:  %s", filename.c_str());
   TiXmlDocument map(filename.c_str());  //Create a new xml doc object from filename
   loadedMapOkay = map.LoadFile();   //Load that file, recording success in loadedMap

   if (loadedMapOkay) {              //if - loading map was successful, we create all elements from the map
      Logger::getLogger()->debug("Loaded %s", filename.c_str());

      TiXmlElement *pRoot, *pG;

      loadedMapOkay = false;        //reset loaded map okay so we can ensure a map gets loaded
      //Get the root svg element
      pRoot = map.FirstChildElement("svg");
      if ( pRoot ) {

         //Get the root g element
         pG = pRoot->FirstChildElement("g");

         if (pG) {
            loadedMapOkay = processGTag(pG, universe);   //map loading must reach this point to be successful
         }// </g>

      }// </svg>

   }

   return loadedMapOkay;             //return true if map was created successfully, otherwise false
}

bool MapImport::processGTag(TiXmlElement* pG, IGObject& universe) {
   bool result = true;
   
   IGObject::Ptr wormholes      = createConstellation(universe, "Wormholes", 0); //Place to put wormholes

   //A map used to relate the label of a constellation to the IGObject::Ptr 
   std::map<string,IGObject::Ptr > labelToPlanet;

   TiXmlElement* pPath;

   //Process Rectangles in g
   Logger::getLogger()->debug("Starting on rects processing");
   processRectTag(pG,universe,labelToPlanet);
   Logger::getLogger()->debug("Finished with rects processing");

   //Process Paths in g
   pPath = pG->FirstChildElement("path");
   Logger::getLogger()->debug("Starting on paths processing");
   while (pPath) {
      string p1 = pPath->Attribute("inkscape:connection-start");
      string p2 = pPath->Attribute("inkscape:connection-end");
      
      //Process each individual Path and translate into graph
      Logger::getLogger()->debug("Looking at a path, start is: %s, end is %s",
         p1.c_str(),p2.c_str());
      
      if ((p1 != "" && p2 != "") && labelToPlanet[p1]!=NULL && labelToPlanet[p2]!= NULL)
         createWormhole(*wormholes,labelToPlanet[p1],labelToPlanet[p2]);

      //Get the next path
      pPath = pPath->NextSiblingElement("path");
   }
   Logger::getLogger()->debug("Finished with paths processing");
   
   return result;
}

//Process each individual Rectangle and translate to Star
bool MapImport::processRectTag(TiXmlElement* pG, IGObject& universe, std::map<string,IGObject::Ptr >& labelToPlanet) {
   bool result = true;
   
   TiXmlElement* pRect;
   pRect = pG->FirstChildElement("rect");
   
   //A map used to relate the colors of a constellation to the IGObject::Ptr 
   std::map<string,IGObject::Ptr > fillToConstellation;
   
   while (pRect) {
      Logger::getLogger()->debug("Got rect, id is: %s",pRect->Attribute("id"));
      IGObject::Ptr parent;

      string fill;
   
      double rectX;
      double rectY;
      string name;

      //Extract the fill from the style attribute
      fill = getFillFromStyle(pRect->Attribute("style"));
                  
      //Check if constellation exists for given fill color
      std::map<string,IGObject::Ptr >::const_iterator fillExists = fillToConstellation.find(fill);
      if (fillExists == fillToConstellation.end()) { //if color doesn't exist, create constellation for it
         std::pair<string,uint32_t> cnstNameAndBonus = getNameAndBonus(pG,fill);
         parent = createConstellation(universe,cnstNameAndBonus.first,cnstNameAndBonus.second);
         fillToConstellation[fill] = parent;
      }
      else
         parent = fillToConstellation[fill];
      
      //Get doubles out of strings for x and y
      std::istringstream xstream( pRect->Attribute("x"));
   	xstream >> rectX;
      std::istringstream ystream( pRect->Attribute("y"));
   	ystream >> rectY;
      rectY *= -1; //invert the map, since svg files measure from top to bottom

      name = pRect->Attribute("id");
   
      //Create the star system and keep track of the reference in the labelToPlanet map.
      labelToPlanet['#'+name]=createStarSystem(*parent,  removeUnderscores(name), rectX, rectY);
      pRect = pRect->NextSiblingElement("rect");
   }
   
   return result;
}

std::pair<string,uint32_t> MapImport::getNameAndBonus(TiXmlElement* pG, string fill) {
   TiXmlElement *pText, *pTSpan;
   std::pair<string,uint32_t> result;
   result.first = ""; result.second = 0;

   pText = pG->FirstChildElement("text");   
   while(pText) {
      pTSpan = pText->FirstChildElement("tspan");
      while(pTSpan) {
         string textFill = getFillFromStyle(pTSpan->Attribute("style"));
         if (textFill == fill) { //both colors are identical
            string wholeText = pTSpan->GetText();
            size_t delimiter = wholeText.find("|");
            if ( delimiter != string::npos ) {
               result.first = wholeText.substr(0,delimiter);
               
               std::istringstream bonusstream(wholeText.substr(delimiter+1));
            	bonusstream >> result.second;
            }
         }
            
         pTSpan = pTSpan->NextSiblingElement("tspan");
      }
      
      pText = pText->NextSiblingElement("text");
   }
   
   //if no name was given, set fill to color
   if (result.first == "")
      result.first = fill;
      
   return result;
}

//Return the hexcode color from a style by looking for fill:# and taking the next 6 chars after that
string MapImport::getFillFromStyle(string longStyle) {
   size_t fillPosn;
   string result = "";
   int styleTrimLen = 6;   //The length of text to trim from the style to get the hexcode color
   int styleHexLen = 6;    //The length of text to extract fromthe style to get the hexcode color
   
   //Logger::getLogger()->debug("Style is initially: %s",longStyle.c_str());
   
   fillPosn = longStyle.find("fill:#"); //Find where the fill occurs in the style string
   if (fillPosn != string::npos)
      result = longStyle.substr(fillPosn+styleTrimLen,styleHexLen);
   
   //Logger::getLogger()->debug("Style was detected to be %s",result.c_str());
   
   return result;
}

// Herschell_s_Garnet_Star will become: 
// Herschell's Garnet Star
string MapImport::removeUnderscores(string str) {
   int size = str.size();
   
   for (int i = 0; i < size; i++) {
      if (str[i] == '_')
      {
         str[i] = ' ';
         if( i == size - 2 && str[i+1] == 's' ) //we're at the second last char and the last char is 's'
         {
            Logger::getLogger()->debug("got end of line _s, replacing with \'s");
            str[i] = '\'';
         }
         else if( i < size - 2 && str[i+1] == 's' && str[i+2] == '_') {
            Logger::getLogger()->debug("got non-end of line _s, replacing with \'s");
            str[i] = '\'';
         }

      }
   }
   return str;
}

IGObject::Ptr MapImport::createConstellation(IGObject& parent, const string& name, int bonus) {
   DEBUG_FN_PRINT();
   
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject::Ptr constellation = game->getObjectManager()->createNewObject();

   otypeman->setupObject(constellation, otypeman->getObjectTypeByName("Constellation"));
   constellation->setName(name);

   Constellation* constellationData = dynamic_cast<Constellation*>(constellation->getObjectBehaviour());
   constellationData->setBonus(bonus);
   constellationData->setIcon("common/object-icons/system");
   constellationData->setMedia("common-2d/foreign/freeorion/nebula-small/" + constellationmedia->getName());
   constellation->addToParent(parent.getID());
   game->getObjectManager()->addObject(constellation);

   return constellation;
}

//Its VERY important to note that the star system creation function returns a pointer to the PLANET
IGObject::Ptr MapImport::createStarSystem(IGObject& parent, const string& name, double unitX, double unitY) {
   DEBUG_FN_PRINT();
   
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   Risk* risk = dynamic_cast<Risk*>(game->getRuleset());
   Graph* graph = risk->getGraph();

   IGObject::Ptr starSys = game->getObjectManager()->createNewObject();

   otypeman->setupObject(starSys, otypeman->getObjectTypeByName("Star System"));
   starSys->setName(name+" System");
   
   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSys->getObjectBehaviour());
   starSysData->setUnitPos(unitX, unitY);
   starSysData->setIcon("common/object-icons/system");
   starSysData->setMedia("common-2d/star-small/" + systemmedia->getName());
   starSys->addToParent(parent.getID());
   game->getObjectManager()->addObject(starSys);

   //Create the planet AND add that planet to the graph.
   IGObject::Ptr planet = createPlanet(*starSys, name, starSysData->getPosition() + getRandPlanetOffset());
   graph->addPlanet(planet);
   
   return planet;
}

IGObject::Ptr MapImport::createPlanet(IGObject& parent, const string& name,double unitX, double unitY) {
   return createPlanet(parent, name, Vector3d(unitX,unitY,0));
}

IGObject::Ptr MapImport::createPlanet(IGObject& parent, const string& name,const Vector3d& location) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   Risk* risk = dynamic_cast<Risk*>(game->getRuleset());
   
   IGObject::Ptr planet = game->getObjectManager()->createNewObject();
   
   otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));
   planet->setName(name);
   
   Planet* planetData = dynamic_cast<Planet*>(planet->getObjectBehaviour());
   planetData->setPosition(location); // OK because unit pos isn't useful for planets
   planetData->setDefaultResources();
   planetData->setIcon("common/object-icons/planet");
   planetData->setMedia(std::string("common-2d/foreign/freeorion/planet-small/animation/") + planetmedia->getName());
   
   OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*> 
         (planet->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId( game->getOrderManager()->addOrderQueue(planet->getID(),0) );
   planetData->setOrderTypes();
   
   planet->addToParent(parent.getID());
   game->getObjectManager()->addObject(planet);
   
   risk->increaseNumPlanets();
   return planet;
}   

void MapImport::createWormhole(IGObject& parent, int64_t startat, int64_t endat) {
   DEBUG_FN_PRINT();
   
   Game *game = Game::getGame();

   // Add the graph edge for risk..
   Risk* risk = dynamic_cast<Risk*>(Game::getGame()->getRuleset());
   Graph* graph = risk->getGraph();
   graph->addEdge(startat,endat);

   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   ObjectManager* om = game->getObjectManager();

   // Get the start and ending system's so we can pull off their coordinates..
   IGObject::Ptr startSystem = om->getObject(om->getObject(startat)->getParent());
   StaticObject *startSystemData = (StaticObject*)startSystem->getObjectBehaviour();
   IGObject::Ptr endSystem = om->getObject(om->getObject(endat)->getParent());
   StaticObject *endSystemData = (StaticObject*)endSystem->getObjectBehaviour();

   // Create a new wormhole
   IGObject::Ptr wormhole = game->getObjectManager()->createNewObject();
   otypeman->setupObject(wormhole, otypeman->getObjectTypeByName("Wormhole"));

   wormhole->setName(startSystem->getName() + " to " + endSystem->getName());

   Wormhole* wormholeData = dynamic_cast<Wormhole*>(wormhole->getObjectBehaviour());
   wormholeData->setEndA(startSystemData->getPosition());
   wormholeData->setEndB(endSystemData->getPosition());

   wormhole->addToParent(parent.getID());
   game->getObjectManager()->addObject(wormhole);

   game->getObjectManager()->doneWithObject(startat);
   game->getObjectManager()->doneWithObject(endat);
}

void MapImport::createWormhole(IGObject& parent, IGObject::Ptr startat, IGObject::Ptr endat) {
   createWormhole(parent,startat->getID(),endat->getID());
}
}//end namespace RiskRuleset
