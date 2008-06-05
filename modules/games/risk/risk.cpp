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
#include "constellation.h"

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

//Constructor with a initializer for adjacency_list graph, sets graph to have 0 vertices/edges 
Risk::Risk() : graph(), random(NULL) {
   num_constellations = 0;
   num_planets = 0;
}

Risk::~Risk(){
   if(random != NULL){
     delete random;
   }
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
   //Game::getGame()->setRuleset(this);

   setObjectTypes();

   setOrderTypes();
}

void Risk::setObjectTypes() const{
   ObjectTypeManager* obdm = Game::getGame()->getObjectTypeManager();
   StaticObjectType* eo;    

   obdm->addNewObjectType(new UniverseType());

   //old
   /*  eo = new StaticObjectType();
   eo->setTypeName("Constellation");
   eo->setTypeDescription("A set of star systems that provides a bonus of reinforcements to any player completely controlling it.");
   obdm->addNewObjectType(eo); */
   //new
   obdm->addNewObjectType(new ConstellationType());

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
/*
   std::string randomseed = Settings::getSettings()->get("risk_debug_random_seed");
   if( randomseed != ""){
      random = new Random();
      random->seed(atoi(randomseed.c_str()));
   } else {
      random = Game::getGame()->getRandom();
   }
*/
   createResources();
 
   //set up universe (universe->constellations->star sys->planet)
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
//Galaxies are ID 1 through num_constellations 
//Planets Systems are num_constellations+1 through num_constellations*2 - 1. System 'k' is at num_constellations+(2k-1)
//Planets are num_constellations+2 through num_constellations+2*num_planets. Planet 'k' is at num_constellations+2k
   //Helper function starID(constellations,planet #) returns the ID of the star
void Risk::createUniverse() {
   DEBUG_FN_PRINT();

   ObjectManager *objman = Game::getGame()->getObjectManager();
   ObjectTypeManager *otypeman = Game::getGame()->getObjectTypeManager();

   uint32_t uniType = otypeman->getObjectTypeByName("Universe");
   IGObject *universe = objman->createNewObject();

   otypeman->setupObject(universe, uniType);
   universe->setName("The Universe");
   StaticObject* uniData = dynamic_cast<StaticObject*>(universe->getObjectBehaviour());
   uniData->setSize(123456789123ll);
   //The field of view for the universe is approximately -1 to 1 X and 0 to 1 Y.
   uniData->setUnitPos(0,.5);
   objman->addObject(universe);

   //LATER: create some sort of import function to create map from file 
   IGObject *con_one = createConstellation(*universe, "One",     2);
   IGObject *con_two = createConstellation(*universe, "Two",     3);
   
   createStarSystem(*con_one, "alpha",          -0.250, 0.650);   //planet #1
   createStarSystem(*con_one, "beta",           -0.250, 0.350);   //planet #2
   
   createStarSystem(*con_two, "gamma",          +0.250, 0.650);   //planet #3
   createStarSystem(*con_two, "delta",          +0.250, 0.350);   //planet #4

   //TEST: used simply for testing, map creation will be a more robust and semi-hidden process once map import is made
   const uint32_t ALPHA = 2 + 2*1;
   const uint32_t BETA  = 2 + 2*2;
   const uint32_t GAMMA = 2 + 2*3;
   const uint32_t DELTA = 2 + 2*4;

   
   boost::add_edge(ALPHA, BETA, graph);
   boost::add_edge(GAMMA, DELTA, graph);
   boost::add_edge(BETA, DELTA, graph);
   boost::add_edge(GAMMA, ALPHA, graph);   
   
   //The question is: refer to planets as 1 to num_planets or as their actual ids?
   //It will be easier to access them by id but harder to create
   //and vica versa

}

IGObject* Risk::createConstellation(IGObject& parent, const string& name, int bonus) {
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

   ++num_constellations;
   return constellation;
}

IGObject* Risk::createStarSystem(IGObject& parent, const string& name, double unitX, double unitY) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *starSys = game->getObjectManager()->createNewObject();

   otypeman->setupObject(starSys, otypeman->getObjectTypeByName("Star System"));
   starSys->setName(name);
   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSys->getObjectBehaviour());
   starSysData->setUnitPos(unitX, unitY);

   starSys->addToParent(parent.getID());
   game->getObjectManager()->addObject(starSys);

   //CHECK: if commenting this out and changing planetName to name caused any problems
   //string planetName;
   //planetName = starSys->getName();
   createPlanet(*starSys, name, starSysData->getPosition() + getRandPlanetOffset());
   return starSys;
}

IGObject* Risk::createPlanet(IGObject& parent, const string& name,double unitX, double unitY) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();

   IGObject *planet = game->getObjectManager()->createNewObject();

   otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));
   planet->setName(name);
   Planet* planetData = dynamic_cast<Planet*>(planet->getObjectBehaviour());
   planetData->setUnitPos(unitX, unitY);
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

   ++num_planets;
   return planet;
}

IGObject* Risk::createPlanet(IGObject& parent, const string& name,const Vector3d& location) {
   DEBUG_FN_PRINT();
   Game *game = Game::getGame();
   ObjectTypeManager *otypeman = game->getObjectTypeManager();
   
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
   
   ++num_planets;
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
      settings->set("max_players", "6");
  
   if(settings->get("game_length") == "")
      settings->set("game_length", "60");

   if (settings->get("risk_rfc_rate") == "")
      settings->set("risk_rfc_rate", "3");

   if (settings->get("risk_rfc_number") == "")
      settings->set("risk_rfc_number", "1");
  
   if (settings->get("risk_default_planet_armies") == "")
      settings->set("risk_default_planet_armies", "3");

   if (settings->get("risk_rfc_start") == "")
      settings->set("risk_rfc_start", "30");
      
   if (settings->get("risk_randomly_assign_territories") == "" )
      settings->set("risk_randomly_assign_territories", "true");
}

bool Risk::onAddPlayer(Player* player){
      Logger::getLogger()->debug("Risk onAddPlayer"); 
      Game* game = Game::getGame();

      bool canJoin = true;            

      //TODO: implement restrictions for adding players
      /* Commented out for the time being until things start working properly

      uint32_t max_players = atoi(Settings::getSettings()->get("max_players").c_str() );
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
   return false;
}

void Risk::onPlayerAdded(Player* player){
   Logger::getLogger()->debug("Risk onPlayerAdded");
   Settings* settings = Settings::getSettings();

   Message *welcome = new Message();
   welcome->setSubject("Welcome to Risk! Here's a brief reminder of some rules");
   welcome->setBody("<b><u>Turn Order</b></u>:<br />\
                     Part 1: Colonization orders are processed<br />\
                     Part 2: Reinforce orders are processed<br />\
                     Part 3: Non-attack movement orders are processed<br />\
                     Part 4: Attack movement orders are processed<br />\
                     <br /><br />\
                     Once you have colonized a planet your total number<br />\
                     of reinforcements will be displayed as the number minable.<br /><br />");

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
   
   if ( settings->get("risk_randomly_assign_territories") == "true" ) {
      randomlyAssignPlanets(player);
   }
   
   //Create a spot in the reinforcements map for the player and assign starting reinforcements.
   reinforcements[player->getID()] = atoi(Settings::getSettings()->get("risk_rfc_start").c_str() );
 
}

//CHECK: not really assigning planets
void Risk::randomlyAssignPlanets(Player* player) {
   Logger::getLogger()->debug("Attempting to randomly assign planets");
   Game* game = Game::getGame();
   ObjectManager* objM = game->getObjectManager();
   PlayerManager *pm = game->getPlayerManager();

   uint32_t to_be_asgned = num_planets / atoi(Settings::getSettings()->get("max_players").c_str() );
   OwnedObject* planet; 
   while (to_be_asgned > 0) {
      uint32_t planet_number = random->getInRange((uint32_t)1,num_planets); //Check is this in range inclusive?
      planet = dynamic_cast<OwnedObject*>(objM->getObject(num_constellations+2*planet_number)->getObjectBehaviour());   //TODO make this more transparent
      if (planet->getOwner() != 0) {
         planet->setOwner(player->getID());
         to_be_asgned--;
      }

   }
}

uint32_t Risk::getPlayerReinforcements(uint32_t owner) {
   //ASK: do I need to check for out of bounds
   //CHECK: Won't compile as a const function
   return reinforcements[owner];
}

void Risk::setPlayerReinforcements(uint32_t owner, uint32_t units) {
   reinforcements[owner] = units;
}

} //end namespace RiskRuleset
