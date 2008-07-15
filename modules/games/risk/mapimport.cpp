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
#include "tinyxml.h"
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

#include <sstream>
#include <map>
#include <string>

//Tyler's "hacky define :p"
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__PRETTY_FUNCTION__))

namespace RiskRuleset {

using std::string;
using std::map;

bool importMapFromFile(string filename, IGObject& universe){
   bool loadedMapOkay = false;
   Logger::getLogger()->debug("Trying to load:  %s", filename.c_str());
   TiXmlDocument map(filename.c_str());  //Create a new xml doc object from filename
   loadedMapOkay = map.LoadFile();   //Load that file, recording success in loadedMap

   if (loadedMapOkay) {              //if - loading map was successful, we create all elements from the map
      Logger::getLogger()->debug("Loaded %s", filename.c_str());

      TiXmlElement *pRoot, *pG, *pRect, *pPath;

      //Get the root svg element
      pRoot = map.FirstChildElement("svg");
      if ( pRoot ) {

         //Get the root g element
         pG = pRoot->FirstChildElement("g");

         if (pG) {
            //A map used to relate the colors of a constellation to the IGObject*
            std::map<string,IGObject*> styleToConstellation;
            
            //A map used to relate the label of a constellation to the IGObject*
            std::map<string,IGObject*> labelToPlanet;
            
            Risk* risk = dynamic_cast<Risk*>(Game::getGame()->getRuleset());
            Graph* graph = risk->getGraph();

            //Start processing Rectangles in g
            pRect = pG->FirstChildElement("rect");
            Logger::getLogger()->debug("Starting on rects processing");
            while (pRect) {
               //Process each individual Rectangle and translate to Star
               Logger::getLogger()->debug("Got rect, id is: %s",pRect->Attribute("id"));

               string style;
               IGObject* parent;
               size_t fillPosn;
               
               double rectX;
               double rectY;
               string name;

               //Extract the fill from the style attribute
               style = pRect->Attribute("style");
               Logger::getLogger()->debug("Style is initially: %s",style.c_str());
               fillPosn = style.find("fill:#"); //Find where the fill occurs in the style string
               if (fillPosn != string::npos)
                  style = style.substr(fillPosn,12);
               Logger::getLogger()->debug("Style was detected to be %s",style.c_str());               

               std::map<string,IGObject*>::const_iterator cnstExists = styleToConstellation.find(style);
               if (cnstExists == styleToConstellation.end()) {
                  //TODO: extract name and bonus
                  parent = createConstellation(universe,style,0);
                  styleToConstellation[style] = parent;

               }
               else
                  parent = styleToConstellation[style];
                  
               std::istringstream xstream( pRect->Attribute("x"));
            	xstream >> rectX;
               std::istringstream ystream( pRect->Attribute("y"));
            	ystream >> rectY;
               rectY *= -1; //invert the map, since svg files measure from top to bottom

               name = pRect->Attribute("id");
               
               //TODO: Replace universe parent with constellation
               //Create the star system and keep track of the reference in the labelToPlanet map.
               labelToPlanet['#'+name]=createStarSystem(*parent, name, rectX, rectY);

               //Get the next rect
               pRect = pRect->NextSiblingElement("rect");
            }
            Logger::getLogger()->debug("Finished with rects processing");

            //Start processing Paths in g
            pPath = pG->FirstChildElement("path");
            Logger::getLogger()->debug("Starting on paths processing");
            while (pPath) {
               //Process each individual Path and translate into graph
               Logger::getLogger()->debug("Looking at a path, start is: %s, end is %s",
                  pPath->Attribute("inkscape:connection-start"),
                  pPath->Attribute("inkscape:connection-end"));
               
               string p1 = pPath->Attribute("inkscape:connection-start");
               string p2 = pPath->Attribute("inkscape:connection-end");
               graph->addEdge(labelToPlanet[p1],labelToPlanet[p2]);
               //get label for start, end
               //remove # from labels?
               

               //Get the next path
               pPath = pPath->NextSiblingElement("path");
            }
            Logger::getLogger()->debug("Finished with paths processing");
         }// </g>
      }// </svg>

   }

   return loadedMapOkay;             //return true if map was created successfully, otherwise false
}

IGObject* createConstellation(IGObject& parent, const string& name, int bonus) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *constellation = game->getObjectManager()->createNewObject();

   otypeman->setupObject(constellation, otypeman->getObjectTypeByName("Constellation"));
   constellation->setName(name);

   Constellation* constellationData = dynamic_cast<Constellation*>(constellation->getObjectBehaviour());
   constellationData->setBonus(bonus);

   constellation->addToParent(parent.getID());
   game->getObjectManager()->addObject(constellation);

   return constellation;
}

//Its VERY important to note that the star system creation function returns a pointer to the PLANET
IGObject* createStarSystem(IGObject& parent, const string& name, double unitX, double unitY) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   Risk* risk = dynamic_cast<Risk*>(game->getRuleset());
   Graph* graph = risk->getGraph();

   IGObject *starSys = game->getObjectManager()->createNewObject();

   otypeman->setupObject(starSys, otypeman->getObjectTypeByName("Star System"));
   starSys->setName(name+" System");
   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSys->getObjectBehaviour());
   starSysData->setUnitPos(unitX, unitY);

   starSys->addToParent(parent.getID());
   game->getObjectManager()->addObject(starSys);

   //Create the planet AND add that planet to the graph.
   IGObject* planet = createPlanet(*starSys, name, starSysData->getPosition() + getRandPlanetOffset());
   graph->addPlanet(planet);
   
   return planet; //TODO: Redo the way this all works
}

IGObject* createPlanet(IGObject& parent, const string& name,double unitX, double unitY) {
   return createPlanet(parent, name, Vector3d(unitX,unitY,0));
}

IGObject* createPlanet(IGObject& parent, const string& name,const Vector3d& location) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   Risk* risk = dynamic_cast<Risk*>(game->getRuleset());
   
   IGObject *planet = game->getObjectManager()->createNewObject();
   
   otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));
   planet->setName(name);
   Planet* planetData = dynamic_cast<Planet*>(planet->getObjectBehaviour());
   planetData->setPosition(location); // OK because unit pos isn't useful for planets
   planetData->setDefaultResources();
   
   OrderQueue *planetOrders = new OrderQueue();
   planetOrders->setObjectId(planet->getID());
   game->getOrderManager()->addOrderQueue(planetOrders);
   OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*> 
         (planet->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(planetOrders->getQueueId());
   planetData->setOrderTypes();
   
   planet->addToParent(parent.getID());
   game->getObjectManager()->addObject(planet);
   
   risk->increaseNumPlanets();
   return planet;
}   

}//end namespace RiskRuleset
