/*  Risk rulesset class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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
 
// System includes 
#include <sstream> 

// tpserver includes 
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>

#include <tpserver/property.h>
#include <tpserver/component.h>
#include <tpserver/design.h>
#include <tpserver/designview.h>
#include <tpserver/category.h>
#include <tpserver/designstore.h>

#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/prng.h>
#include <tpserver/settings.h>
#include <tpserver/message.h>

// risk includes 
#include "risk.h"
#include "riskturn.h" 
#include "colonize.h"
#include "move.h"
#include "reinforce.h"
#include "containertypes.h"
#include "map.h"
#include "staticobject.h"
#include "universe.h"
#include "ownedobject.h"
#include "planet.h"
#include "galaxy.h"

//Tyler's "hacky define :p"
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__PRETTY_FUNCTION__))

//the libtool magic required
extern "C" { 
   #define tp_init librisk_LTX_tp_init 
   bool tp_init(){ 
      return Game::getGame()->setRuleset(new RiskRuleset::Risk());
   } 
}

namespace RiskRuleset {
  
using std::string;
using std::map;
using std::set;
using std::vector;
using std::advance;
using std::pair;

//TODO: Dynamically resize matrix to number of planets - 42 is an arbitrary number (default map # of planets)
//It appears as if adjacency list, the likely replacement for matrix, supports no size initilization
Risk::Risk() /*:matrix(42) */{
   //Minisec has a parent of random(NULL), whats with that?	
   
}

Risk::~Risk(){
   //minisec simply deletes the random if not NULL    
}

std::string Risk::getName(){
   return "Risk";
}

std::string Risk::getVersion(){
   return "0.1";
}

void Risk::initGame(){
   Logger::getLogger()->info("Risk initialised");

   Game::getGame()->setTurnProcess(new RiskTurn());

   setObjectTypes();

   setOrderTypes();
}

void Risk::setObjectTypes() const{
   ObjectTypeManager* obdm = Game::getGame()->getObjectTypeManager();
   StaticObjectType* eo;    

   obdm->addNewObjectType(new UniverseType());

   //old
   /*  eo = new StaticObjectType();
   eo->setTypeName("Galaxy");
   eo->setTypeDescription("A set of star systems that provides a bonus of reinforcements to any player completely controlling it.");
   obdm->addNewObjectType(eo); */
   //new
   obdm->addNewObjectType(new GalaxyType());

   eo = new StaticObjectType();
   eo->setTypeName("Star System");
   eo->setTypeDescription("A territory capable of being controlled and having any number of armies.");
   obdm->addNewObjectType(eo);

   obdm->addNewObjectType(new PlanetType);   //may need to special some stuff here
   //There are no fleets in risk - hence no fleet type
}

void Risk::setOrderTypes() const{
   OrderManager* orm = Game::getGame()->getOrderManager();

   //To be an action availible on all unowned planets
   //With planet selected order is to colonize with NUMBER armies
   orm->addOrderType(new Colonize());

   //To be an action availible on all player owned planets
   //With planet selected order is to reinforce with NUMBER armies
   orm->addOrderType(new Reinforce());

   //To be an action availible on all player owned planets
   //With planet selected order is to reinforce with NUMBER armies
   orm->addOrderType(new Move());
}

void Risk::createGame(){
   Logger::getLogger()->info("Risk created");

   createResources();
 
   //set up universe (universe->galaxies->star sys->planet)
   createUniverse();
}

void Risk::createResources() {
   ResourceManager* resMan = Game::getGame()->getResourceManager();

   ResourceDescription* res = new ResourceDescription();
   res->setNameSingular("Army");
   res->setNamePlural("Armies");
   res->setUnitSingular("unit");
   res->setUnitPlural("units");
   res->setDescription("Armies");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);
}

//Universe is ID 0
//Galaxies are ID #Planets + 1 and up
//Planets  are ID 1 - num_planets
//Planets systems are num_planets*2 + Planet ID
void Risk::createUniverse() {
   DEBUG_FN_PRINT();
   num_planets = 42;    //CHECK: This may not be the best way to "get/set" the total # of planets
   ObjectManager *objman = Game::getGame()->getObjectManager();
   ObjectTypeManager *otypeman = Game::getGame()->getObjectTypeManager();

   uint32_t uniType = otypeman->getObjectTypeByName("Universe");
   IGObject *universe = objman->createNewObject();

   //TODO: Adjust default position of universe
   //TODO: Perhaps push constellations away from eachother for visibility 
   otypeman->setupObject(universe, uniType);
   universe->setName("The Universe");
   universe->setID(0);
   StaticObject* uniData = static_cast<StaticObject*>(universe->getObjectBehaviour());
   uniData->setSize(123456789123ll);
   uniData->setUnitPos(.5,.5);
   objman->addObject(universe);

   //LATER: create some sort of import function to create map from file 
   //create galaxies and keep reference for system creation
   IGObject *gal_cassiopeia = createGalaxy(*universe, "Cassiopeia", 5, 1 + num_planets); //North America
   IGObject *gal_cygnus     = createGalaxy(*universe, "Cygnus", 2, 2 + num_planets); //South America
   IGObject *gal_cepheus    = createGalaxy(*universe, "Cepheus", 5, 3 + num_planets); //Europe
   IGObject *gal_orion      = createGalaxy(*universe, "Orion",3, 4 + num_planets); //Africa
   IGObject *gal_draco      = createGalaxy(*universe, "Draco", 7, 5 + num_planets); //Russia
   IGObject *gal_crux       = createGalaxy(*universe, "Crux Australis", 2, 6 + num_planets); //Australia

   Logger::getLogger()->info("Galaxies Created");

   //create systems
   // Cassiopeia Systems (North America, Bonus 5)
   createStarSystem(*gal_cassiopeia, "Shedir",         -0.321, 0.670, 1);
   createStarSystem(*gal_cassiopeia, "Caph",           -0.213, 0.751, 2);
   createStarSystem(*gal_cassiopeia, "Ruchbah",        -0.447, 0.724, 3);
   createStarSystem(*gal_cassiopeia, "Gamma Cas",      -0.339, 0.760, 4);
   createStarSystem(*gal_cassiopeia, "Segin",          -0.519, 0.807, 5);
   createStarSystem(*gal_cassiopeia, "Zeta Cas",       -0.303, 0.571, 6);
   createStarSystem(*gal_cassiopeia, "Marfak",         -0.420, 0.616, 7);
   createStarSystem(*gal_cassiopeia, "Xi Cas",         -0.357, 0.481, 8);  
   createStarSystem(*gal_cassiopeia, "Sigma Cas",      -0.222, 0.643, 9);

   // Cygnus Systems (South America, Bonus 2)
   createStarSystem(*gal_cygnus, "Deneb",              -0.321, 0.273, 10);
   createStarSystem(*gal_cygnus, "Albireo",            -0.249, -0.051, 11);
   createStarSystem(*gal_cygnus, "Sadr",               -0.294, 0.156, 12);
   createStarSystem(*gal_cygnus, "Gienah Cygni",       -0.402, 0.138, 13);
 
   // Cepheus Systems (Europe, Bonus 5)
   createStarSystem(*gal_cepheus, "Alderamin",         0.045,  0.472, 14);
   createStarSystem(*gal_cepheus, "Alfirk",            0.063,  0.625, 15);
   createStarSystem(*gal_cepheus, "Al Kalb al Rai",    -0.018, 0.724, 16);
   createStarSystem(*gal_cepheus, "Alrai",             -0.081, 0.724, 17);
   createStarSystem(*gal_cepheus, "The Garnet Star",   -0.045, 0.445, 18);
   createStarSystem(*gal_cepheus, "Alkurhah",          -0.036, 0.499, 19);
   createStarSystem(*gal_cepheus, "Iota Cep",          -0.090, 0.598, 20);

   // Orion Systens (Africa, Bonus 3)
   createStarSystem(*gal_orion, "Betelgeuse",          0.031, 0.228, 21);
   createStarSystem(*gal_orion, "Rigel",               0.226, -0.006, 22);
   createStarSystem(*gal_orion, "Bellatrix",           0.184, 0.237, 23);
   createStarSystem(*gal_orion, "Mintaka",             0.148, 0.120, 24);
   createStarSystem(*gal_orion, "Alnitak",             0.085, 0.102, 25);
   createStarSystem(*gal_orion, "Saiph",               0.085, -0.042, 26);

   // Draco Systems (Russia, Bonus 7)
   createStarSystem(*gal_draco, "Etamin",              0.247, 0.382, 27);
   createStarSystem(*gal_draco, "Rastaban",            0.346, 0.382, 28);
   createStarSystem(*gal_draco, "Arrakis",             0.400, 0.402, 29);
   createStarSystem(*gal_draco, "Kuma",                0.346, 0.436, 30);
   createStarSystem(*gal_draco, "Grumium",             0.247, 0.454, 31);
   createStarSystem(*gal_draco, "Nodus Secundus",      0.211, 0.634, 32);
   createStarSystem(*gal_draco, "Tyl",                 0.202, 0.697, 33);
   createStarSystem(*gal_draco, "Dsibin",              0.304, 0.670, 34);
   createStarSystem(*gal_draco, "Aldhibah",            0.373, 0.544, 35);
   createStarSystem(*gal_draco, "Ed Asiach",           0.499, 0.472, 36);
   createStarSystem(*gal_draco, "Thubah",              0.544, 0.634, 37);
   createStarSystem(*gal_draco, "Gianfar",             0.598, 0.778, 38);

   // Crux Systens (Australia, Bonus 2)
   createStarSystem(*gal_crux, "Acrux",                0.606, 0.000, 39);
   createStarSystem(*gal_crux, "Becrux",               0.466, 0.100, 40);
   createStarSystem(*gal_crux, "Gacrux",               0.534, 0.252, 41);
   createStarSystem(*gal_crux, "Delta Cru",            0.690, 0.161, 42); 
   
   //Construct Adjacency Matrix
   //ASK: how to include Boost/Graphs
   //boost::adjacency_matrix<boost::undirectedS> adjMatrix;
   
}

IGObject* Risk::createGalaxy(IGObject& parent, const string& name, int bonus, uint32_t id) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *galaxy = game->getObjectManager()->createNewObject();

   otypeman->setupObject(galaxy, otypeman->getObjectTypeByName("Galaxy"));
   galaxy->setName(name);
   galaxy->setID(id);

   Galaxy* galaxyData = static_cast<Galaxy*>(galaxy->getObjectBehaviour());
   galaxyData->setBonus(bonus);

   galaxy->addToParent(parent.getID());
   game->getObjectManager()->addObject(galaxy);

   return galaxy;
}

IGObject* Risk::createStarSystem(IGObject& parent, const string& name, double unitX, double unitY, uint32_t id) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *starSys = game->getObjectManager()->createNewObject();

   otypeman->setupObject(starSys, otypeman->getObjectTypeByName("Star System"));
   starSys->setName(name);
   starSys->setID(id+2*num_planets);
   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSys->getObjectBehaviour());
   starSysData->setUnitPos(unitX, unitY);

   starSys->addToParent(parent.getID());
   game->getObjectManager()->addObject(starSys);

   //CHECK: Do i really need to regrab the name?
   string planetName;
   planetName = starSys->getName();
   createPlanet(*starSys, planetName, starSysData->getPosition() + getRandPlanetOffset(), id);
   return starSys;
}
IGObject* Risk::createPlanet(IGObject& parent, const string& name,double unitX, double unitY, uint32_t id) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *planet = game->getObjectManager()->createNewObject();

   otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));
   planet->setName(name);
   planet->setID(id);
   Planet* planetData = static_cast<Planet*>(planet->getObjectBehaviour());
   planetData->setUnitPos(unitX, unitY);
   planetData->setDefaultResources();

   OrderQueue *planetOrders = new OrderQueue();
   planetOrders->setObjectId(planet->getID());
   planetOrders->addOwner(0);
   game->getOrderManager()->addOrderQueue(planetOrders);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>
                                        (planet->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(planetOrders->getQueueId());
   planetData->setOrderTypes();

   planet->addToParent(parent.getID());
   game->getObjectManager()->addObject(planet);

   return planet;
}

IGObject* Risk::createPlanet(IGObject& parent, const string& name,const Vector3d& location, uint32_t id) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   
   IGObject *planet = game->getObjectManager()->createNewObject();
   
   otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));
   planet->setName(name);
   planet->setID(id);
   Planet* planetData = static_cast<Planet*>(planet->getObjectBehaviour());
   planetData->setPosition(location); // OK because unit pos isn't useful for planets
   planetData->setDefaultResources();
   
   OrderQueue *planetOrders = new OrderQueue();
   planetOrders->setObjectId(planet->getID());
   planetOrders->addOwner(0);
   game->getOrderManager()->addOrderQueue(planetOrders);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*> 
         (planet->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(planetOrders->getQueueId());
   planetData->setOrderTypes();
   
   planet->addToParent(parent.getID());
   game->getObjectManager()->addObject(planet);
   
   return planet;
}   

void Risk::startGame(){
   Logger::getLogger()->info("Risk started");

   //Establish some defaults if user does not specify any in config
   Settings* settings = Settings::getSettings();
   if(settings->get("turn_length_over_threshold") == "")
   {
      settings->set("turn_length_over_threshold", "180");
      settings->set("turn_player_threshold", "0");
      settings->set("turn_length_under_threshold", "180");
   }

   if(settings->get("max_players") == "")
      settings->set("max_players", "4");
  
   if(settings->get("game_length") == "")
      settings->set("game_length", "60");
}

bool Risk::onAddPlayer(Player* player){
      Logger::getLogger()->debug("Risk onAddPlayer"); 
      Game* game = Game::getGame();

      bool canJoin = true;            

      /* Commented out for the time being until things start working properly

      uint32_t max_players = atoi(Settings::getSettings()->get("risk_max_players").c_str() );
      bool isStarted = game->isStarted();
      uint32_t cur_players = game->getPlayerManager()->getNumPlayers();

      //If ( max players exceeded OR (game's started AND there are no open spaces))    
         //disallow joining
      if ( (cur_players >= max_players)||(isStarted && !isBoardClaimed()) )
      canJoin = false;  
      else
      canJoin = true;
      */
      return canJoin; 
}

bool Risk::isBoardClaimed() const{
   //This method will run through board and check if all territories
    //are claimed or not

   //TODO: Check board to determine "claimedness"
   //ASK: how to iterate over whole board
   return false;
}

void Risk::onPlayerAdded(Player* player){
   Logger::getLogger()->debug("Risk onPlayerAdded");

   Message *welcome = new Message();
   welcome->setSubject("Welcome to Risk! Here's a brief reminder of some rules");
   welcome->setBody("<b><u>3 Turn Order</b></u>:<br />\
                     Part 1: Colonization orders are processed<br />\
                     Part 2: Reinforce orders are processed<br />\
                     Part 3: Non-attack movement orders are processed<br />\
                     Part 4: Attack movement orders are processed<br />\
                     *repeat*<br /><br />");

   player->postToBoard(welcome);

   Game::getGame()->getPlayerManager()->updatePlayer(player->getID()); 

   //This should make every object visible to an added player
   PlayerView* playerview = player->getPlayerView();
   std::set<uint32_t> objids = Game::getGame()->getObjectManager()->getAllIds();
   for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end();
         ++itcurr){
      ObjectView* obv = new ObjectView();
      obv->setObjectId(*itcurr);
      obv->setCompletelyVisible(true);
    playerview->addVisibleObject(obv);
   }
 
}

} //end namespace RiskRuleset
