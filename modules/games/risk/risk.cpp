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
#include "wormhole.h"
#include "constellation.h"
#include "graph.h"
#include "mapimport.h"

#include "boost/format.hpp"

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
using boost::format;

//Constructor with a initializer for adjacency_list graph, sets graph to have 0 vertices/edges 
Risk::Risk() : graph(), random(NULL) {
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

   setObjectTypes();

   setOrderTypes();
}

void Risk::setObjectTypes() const{
   ObjectTypeManager* obdm = Game::getGame()->getObjectTypeManager();

   obdm->addNewObjectType(new UniverseType());
   obdm->addNewObjectType(new ConstellationType());
   obdm->addNewObjectType(new StaticObjectType("Star System","A territory capable of being controlled and having any number of armies.") );
   obdm->addNewObjectType(new PlanetType());
   obdm->addNewObjectType(new StaticObjectType("",""));
   obdm->addNewObjectType(new WormholeType());
}

void Risk::setOrderTypes() const{
   OrderManager* orm = Game::getGame()->getOrderManager();

   orm->addOrderType(new Colonize());
   orm->addOrderType(new Reinforce());
   orm->addOrderType(new Move());
}

void Risk::createGame(){
   Logger::getLogger()->info("Risk created");

   //Seed the random to be used as the supplied debug random seed (if supplied)
   std::string randomseed = Settings::getSettings()->get("risk_debug_random_seed");
   if( randomseed != ""){
      random = new Random();
      random->seed(atoi(randomseed.c_str()));
   } else {
      random = Game::getGame()->getRandom();
   }

   createResources();
 
   createUniverse();
}

void Risk::createResources() {
  ResourceManager::Ptr resMan = Game::getGame()->getResourceManager();

  ResourceDescription::Ptr res( new ResourceDescription() );
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
void Risk::createUniverse() {
   DEBUG_FN_PRINT();

   ObjectManager *objman = Game::getGame()->getObjectManager();
   ObjectTypeManager *otypeman = Game::getGame()->getObjectTypeManager();

   uint32_t uniType = otypeman->getObjectTypeByName("Universe");
   IGObject::Ptr universe = objman->createNewObject();

   otypeman->setupObject(universe, uniType);
   universe->setName("The Universe");
   
   StaticObject* uniData = dynamic_cast<StaticObject*>(universe->getObjectBehaviour());
   uniData->setSize(123456789123ll);
   //The field of view for the universe is approximately -1 to 1 X and 0 to 1 Y.
   uniData->setUnitPos(-0.1,0.1);
   uniData->setIcon("common/object-icons/system");
   uniData->setMedia("common-2d/foreign/freeorion/nebula-small/nebula3");
   objman->addObject(universe);
   
   //set up universe (universe->constellations->star sys->planet)
   std::string risk_mapimport = Settings::getSettings()->get("risk_mapimport");
   if( risk_mapimport == "true") {
      std::string mapfile = Settings::getSettings()->get("risk_map");
      MapImport mp;
      bool map_loaded = mp.importMapFromFile(mapfile,*universe);
      if(!map_loaded){
          Logger::getLogger()->error("Could not load mapfile: %s", mapfile.c_str());
          Logger::getLogger()->info("Loading test system instead");
          createTestSystems(*universe);
      }
   }
   else {
      createTestSystems(*universe);
   }
}

void Risk::createTestSystems(IGObject& universe) {

   MapImport mp;
    
   IGObject::Ptr con_cygnus     = mp.createConstellation(universe, "Cygnus",         2); //South America
   IGObject::Ptr con_orion      = mp.createConstellation(universe, "Orion",          3); //Africa
   IGObject::Ptr wormholes      = mp.createConstellation(universe, "Wormholes",      0); //Place to put the wormholes

   // Cygnus Systems (South America, Bonus 2)
   mp.createStarSystem(*con_cygnus, "Deneb",              -0.321, 0.273);  //4
   mp.createStarSystem(*con_cygnus, "Albireo",            -0.249, -0.051); //6
   mp.createStarSystem(*con_cygnus, "Sadr",               -0.294, 0.156);  //8
   mp.createStarSystem(*con_cygnus, "Gienah Cygni",       -0.402, 0.138);  //10

   // Orion Systens (Africa, Bonus 3)
   mp.createStarSystem(*con_orion, "Betelgeuse",          0.031, 0.228);   //12
   mp.createStarSystem(*con_orion, "Rigel",               0.226, -0.006);  //14
   mp.createStarSystem(*con_orion, "Bellatrix",           0.184, 0.237);   //16
   mp.createStarSystem(*con_orion, "Mintaka",             0.148, 0.120);   //18
   mp.createStarSystem(*con_orion, "Alnitak",             0.085, 0.102);   //20
   mp.createStarSystem(*con_orion, "Saiph",               0.085, -0.042);  //22
   
   //Cygnus Internal Adjacencies
   mp.createWormhole(*wormholes, 5, 9);
   mp.createWormhole(*wormholes, 5, 11);
   mp.createWormhole(*wormholes, 9, 11);
   mp.createWormhole(*wormholes, 9, 7);
   mp.createWormhole(*wormholes, 7, 11);
   
   //Orion Internal Adjacencies
   mp.createWormhole(*wormholes, 13, 17);
   mp.createWormhole(*wormholes, 13, 21);
   mp.createWormhole(*wormholes, 17, 19);
   mp.createWormhole(*wormholes, 19, 21);
   mp.createWormhole(*wormholes, 19, 15);
   mp.createWormhole(*wormholes, 21, 23);
   mp.createWormhole(*wormholes, 23, 15);
   
   //Cygnus - Orion Adjacencies
   mp.createWormhole(*wormholes, 13, 9);
}

void Risk::startGame(){
   Logger::getLogger()->info("Risk started");
   
   setDefaults();
}

void Risk::setDefaults() {
   Logger::getLogger()->debug("=Setting Defaults=");
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
   Logger::getLogger()->debug("risk_randomly_assign=%s",settings->get("risk_randomly_assign").c_str());   
   if (settings->get("risk_randomly_assign") == "" )
   {
      Logger::getLogger()->debug("risk_randomly_assign was %s and is now set to \"true\"",settings->get("risk_randomly_assign").c_str());
      settings->set("risk_randomly_assign", "true");
   }
     
   if (settings->get("risk_attack_dmg") == "" )
      settings->set("risk_attack_dmg","1");
      
   if (settings->get("risk_randomly_assign") == "true" ) {
      settings->set("risk_allow_colonize","false");
   }
   else
      settings->set("risk_allow_colonize","true");
      
   if (settings->get("risk_mapimport") == "" ) 
      settings->set("risk_mapimport","false");
      
   if (settings->get("risk_map") == "")
      settings->set("risk_map","risk-defaultmap.svg");
}

bool Risk::onAddPlayer(Player::Ptr player){
      Logger::getLogger()->debug("Risk onAddPlayer"); 
      Game* game = Game::getGame();

      bool canJoin = true;            

      uint32_t max_players = atoi(Settings::getSettings()->get("max_players").c_str() );
      uint32_t cur_players = game->getPlayerManager()->getNumPlayers();

      //If ( max players exceeded OR (game's started AND there are no open spaces))    
         //disallow joining
      Logger::getLogger()->debug("There are %d current players and the max players is %d",cur_players,max_players);
      //joins before other players then he takes a spot
      

      if ( player->getName() == "guest") {
         max_players++;    //increment max players for the meantime
          format newMax("%1%"); newMax % max_players;
         Settings::getSettings()->set("max_players",newMax.str().c_str()); //increase the recorded maximum # of players
      }

      if ( ( (cur_players >= max_players) || isBoardClaimed() == true ) ) {
         canJoin = false;  
      }
      
      return canJoin; 
}

bool Risk::isBoardClaimed() const{
   Logger::getLogger()->debug("Risk::isBoardClaimed has been called");
   Game* game = Game::getGame();
   ObjectManager* objM = game->getObjectManager();

   //Get all objects frobjM object manager
   set<uint32_t> objectsIds = objM->getAllIds();
   set<uint32_t> owners;
   uint32_t owner;
   bool result = true;  //return value
   
   //Iterate over every object
   for(set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i)
   {  
      //Get an object
      IGObject::Ptr currObj = objM->getObject(*i);
      OwnedObject *ownedObj = dynamic_cast<OwnedObject*>(currObj->getObjectBehaviour());

      if ( ownedObj != NULL) {         //if the object IS an owned objectowned
         owner = ownedObj->getOwner(); 
            owners.insert(owner);      //Add the object's owner to the set
      }
   }
   
   if ( owners.find(0) != owners.end() ) {   //If there is a unowned object in the set
      result = false;               //Change result to indicate there exists an OwnedObject w/o an owner
      Logger::getLogger()->debug("Risk::isBoardClaimed will return false");      
   }
   else
      Logger::getLogger()->debug("Risk::isBoardClaimed will return true");
   
   return result;
}

void Risk::onPlayerAdded(Player::Ptr player){
   Logger::getLogger()->debug("Risk onPlayerAdded");
   Settings* settings = Settings::getSettings();

   Message::Ptr welcome( new Message() );
   welcome->setSubject("Welcome to Risk! Here's a brief reminder of some rules");
   welcome->setBody("<b><u>Turn Order</b></u>:<br />\
                     Part 1: Colonization orders are processed<br />\
                     Part 2: Reinforce orders are processed<br />\
                     Part 3: Movement orders are processed<br />\
                     <br /><br />");

   player->postToBoard(welcome);

   Game::getGame()->getPlayerManager()->updatePlayer(player->getID()); 

   setPlayerVisibleObjects();

   //Restrict guest player from receiving reinforcements or planets:
   if ( player->getName() != "guest" ) {
      //Create a spot in the reinforcements map for the player and assign starting reinforcements.
      reinforcements[player->getID()] = atoi(Settings::getSettings()->get("risk_rfc_start").c_str() );
   
      Logger::getLogger()->debug("risk_randomly_assign=%s",settings->get("risk_randomly_assign").c_str());
      if ( settings->get("risk_randomly_assign") == "true") {
         Logger::getLogger()->debug("Starting a random assignment game");
         randomlyAssignPlanets(player);
      }
      else {
         Logger::getLogger()->debug("Starting a bidding game");
         randomlyGiveOnePlanet(player);
      }
   }
}

void Risk::randomlyAssignPlanets(Player::Ptr player) {
   Logger::getLogger()->debug("Starting fractional random planet assignment for player %d", player->getID());

   //get applicable settings
   uint32_t max_players = atoi(Settings::getSettings()->get("max_players").c_str() );

   //Round is like this so there is a possability a few open spaces to exist
   uint32_t to_be_asgned = num_planets / max_players;
   
   randomlyPickPlanets(player,to_be_asgned);
}

void Risk::randomlyGiveOnePlanet(Player::Ptr player) {
   Logger::getLogger()->debug("Starting single random planet assignment for player %d", player->getID());

   //Randomly give the player planet
   randomlyPickPlanets(player,1);
}

void Risk::randomlyPickPlanets(Player::Ptr player, uint32_t numPlanets) {
   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();
   
   std::string body = "";
   
   uint32_t armies = atoi(Settings::getSettings()->get("risk_default_planet_armies").c_str() );
   
   Logger::getLogger()->debug("The number of players to be assigned is %d.", numPlanets);
   
   Planet* planet; 
   std::set<uint32_t> allIds = om->getAllIds();
   std::set<uint32_t> unownedObjs;
   
   //This for loop populates set unownedObjs with only objects with no owner.
   for (std::set<uint32_t>::iterator i = allIds.begin(); i != allIds.end(); ++i) {     //Iterate over all ids
      IGObject::Ptr obj = om->getObject(*i);
      Planet* ownedObj = dynamic_cast<Planet*>(obj->getObjectBehaviour());
      
      if (ownedObj != NULL && ownedObj->getOwner() == 0 ) {
            unownedObjs.insert(ownedObj->getID());            //Insert only unowned "OwnedObject" id into set
      }
   }
   
   //This loop gives out planets until all planets to be assigned are gone or no planets remain open
   while (numPlanets > 0 && !isBoardClaimed() ) { 
      Logger::getLogger()->debug("Starting to assign random planets to player %d", player->getID());

      uint32_t planet_number = random->getInRange((uint32_t)0,unownedObjs.size()-1);

      //get and move iterator it to the planet number randomly chosen
      std::set<uint32_t>::iterator it = unownedObjs.begin(); 
      for ( uint32_t i = 0; i < planet_number; i++, it++) {}
      
      //The position of iterator is a randomly chosen unowned object
      uint32_t id_to_grab = *(it);

      planet = dynamic_cast<Planet*>(om->getObject(id_to_grab)->getObjectBehaviour());
      
      Logger::getLogger()->debug("Picked planet #%d to give out", id_to_grab);
      
      if (planet != NULL && planet->getOwner() <=  0) {                          //if planet is not owned
         planet->setOwner(player->getID());                                      //let the player have it
         planet->setResource("Army", armies, reinforcements[player->getID()]);   //and update availible resources with defaults
         planet->setOrderTypes();
         numPlanets--;
         
         body += "You received the planet "+planet->getName()+".<br />";
      }

   }
   
   //Construct and send message regarding assigned planets
   Message::Ptr gotPlanet( new Message() );
   if (body != "") {
      gotPlanet->setSubject("Your received a randomly assigned planet(s)");
      gotPlanet->setBody(body);
   }
   else {
      gotPlanet->setSubject("You have not received any planets");
      gotPlanet->setBody("This may mean there were no planets to give out, or an error has occured");
   }
   player->postToBoard(gotPlanet);
}

Graph* Risk::getGraph() {
   return &graph;
}

uint32_t Risk::getPlayerReinforcements(uint32_t owner) {
   uint32_t result = 0;
   if (reinforcements.find(owner) != reinforcements.end())
      result = (*reinforcements.find(owner)).second;
   return result;
}

void Risk::setPlayerReinforcements(uint32_t owner, uint32_t units) {
   reinforcements[owner] = units;
}

void Risk::increaseNumPlanets() {
   ++num_planets;
}

} //end namespace RiskRuleset
