/*  MiniSec ruleset
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <cassert>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/ownedobject.h>
#include <tpserver/universe.h>
#include <tpserver/emptyobject.h>
#include "planet.h"
#include "fleet.h"
#include <tpserver/objectdatamanager.h>
#include <tpserver/player.h>
#include "rspcombat.h"
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include <tpserver/nop.h>
#include "move.h"
#include "build.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"
#include <tpserver/property.h>
#include <tpserver/component.h>
#include <tpserver/design.h>
#include <tpserver/category.h>
#include <tpserver/logging.h>
#include <tpserver/playermanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/settings.h>
#include <tpserver/prng.h>

#ifdef HAVE_LIBMYSQL
#include <modules/persistence/mysql/mysqlpersistence.h>
#include <modules/persistence/mysql/mysqluniverse.h>
#include "mysqlemptyobject.h"
#include "mysqlplanet.h"
#include "mysqlfleet.h"
#endif

#include "minisec.h"

static char const * const systemNames[] = {
    "Barnard's Star",  "Gielgud",             "Ventana",
    "Aleph Prime",     "Ventil",              "Sagitaria",
    "Drifter",         "Ptelemicus",          "Centanis",
    "Mendelis",        "Cassious' Shadow",    "Llentim",
    "Redoubt",         "Kelper",              "Cemara",
    "Cilantarius",     "Kya",                 "Lanternis",
    "Illatis",         "Rintim",              "Uvaharim",
    "Plaetais",        "Denderis",            "Desiderata",
    "Illuntara",       "Ivemteris",           "Wetcher",
    "Monanara",        "Clesasia",            "RumRunner",
    "Last Chance",     "Kiuper Shadow",       "NGC 42059",
    "Ceti Alpha",      "Surreptitious",       "Lupus Fold",
    "Atlantis",        "Draconis",            "Muir's Gold",
    "Fools Errand",    "Wrenganis",           "Humph",
    "Byzantis",        "Torontis",            "Radiant Pool"};


extern "C" {
  bool tp_init(){
    return Game::getGame()->setRuleset(new MiniSec());
  }
}

MiniSec::MiniSec(){

}

MiniSec::~MiniSec(){

}

std::string MiniSec::getName(){
  return "MiniSec";
}

std::string MiniSec::getVersion(){
  return "0.2";
}

void MiniSec::initGame(){
  Game* game = Game::getGame();
  
  
    game->setCombatStrategy(new RSPCombat());
  

  ObjectDataManager* obdm = game->getObjectDataManager();
  obdm->addNewObjectType(new Universe());
  obdm->addNewObjectType(new EmptyObject());
  obdm->addNewObjectType(new EmptyObject());
  obdm->addNewObjectType(new Planet());
  obdm->addNewObjectType(new Fleet());

#ifdef HAVE_LIBMYSQL
    MysqlPersistence* database = dynamic_cast<MysqlPersistence*>(game->getPersistence());
    if(database != NULL){
        MysqlUniverse* uni = new MysqlUniverse();
        uni->setType(0);
        database->addObjectType(uni);
        MysqlEmptyObject* emt = new MysqlEmptyObject();
        emt->setType(1);
        database->addObjectType(emt);
        emt = new MysqlEmptyObject();
        emt->setType(2);
        database->addObjectType(emt);
        MysqlPlanet* plnt = new MysqlPlanet();
        plnt->setType(3);
        database->addObjectType(plnt);
        MysqlFleet* flt = new MysqlFleet();
        flt->setType(4);
        database->addObjectType(flt);
    }
#endif

  OrderManager* ordm = game->getOrderManager();
  ordm->addOrderType(new Nop());
  ordm->addOrderType(new Move());
  ordm->addOrderType(new Build());
  ordm->addOrderType(new Colonise());
  ordm->addOrderType(new SplitFleet());
  ordm->addOrderType(new MergeFleet());

}

void MiniSec::createGame(){
  Game* game = Game::getGame();
  
    DesignStore *ds = game->getDesignStore();
    Category * cat = new Category();
    cat->setName("Ships");
    cat->setDescription("The Ship design and component category");
    ds->addCategory(cat);
    assert(cat->getCategoryId() == 1);
    
    Property* prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Speed");
    prop->setDisplayName("Speed");
    prop->setDescription("The number of units the ship can move each turn");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000000)) \" mega-units\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("BuildTime");
    prop->setDisplayName("Build Time");
    prop->setDescription("The number of turns to build the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" turns\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Armour");
    prop->setDisplayName("Armour");
    prop->setDescription("The amount of amour on the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("WeaponWin");
    prop->setDisplayName("Weapon Strength at Win");
    prop->setDescription("The number of HP to do to the fired at ship when RSP wins");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("WeaponDraw");
    prop->setDisplayName("Weapon Strength at Draw");
    prop->setDescription("The number of HP to do to the fired at ship when RSP draws");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Colonise");
    prop->setDisplayName("Can Colonise Planets");
    prop->setDescription("Can the ship colonise planets");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (if (= n 1) \"Yes\" \"No\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    prop = new Property();
    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("_num-components");
    prop->setDisplayName("Number of Conponents");
    prop->setDescription("The total number of components in the design");
    prop->setTpclDisplayFunction(
        "(lambda (design bits)"
        "(let ((n (apply + bits)))"
        "(cons n (string-append (number->string n) \" components\"))))");
        prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    
    std::map<unsigned int, std::string> propertylist;
    
    Component* comp = new Component();
    comp->setCategoryId(1);
    comp->setName("ScoutHull");
    comp->setDescription("The scout hull, fitted out with everything a scout needs");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[1] = "(lambda (design) 500000000)";
    propertylist[2] = "(lambda (design) 1)";
    propertylist[3] = "(lambda (design) 2)";
    propertylist[4] = "(lambda (design) 0)";
    propertylist[5] = "(lambda (design) 0)";
    propertylist[7] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);
    
    comp = new Component();
    comp->setCategoryId(1);
    comp->setName("FrigateHull");
    comp->setDescription("The frigate hull, fitted out with everything a frigate needs");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist.clear();
    propertylist[1] = "(lambda (design) 200000000)";
    propertylist[2] = "(lambda (design) 2)";
    propertylist[3] = "(lambda (design) 4)";
    propertylist[4] = "(lambda (design) 2)";
    propertylist[5] = "(lambda (design) 0)";
    propertylist[6] = "(lambda (design) 1)";
    propertylist[7] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);
    
    comp = new Component();
    comp->setCategoryId(1);
    comp->setName("BattleshipHull");
    comp->setDescription("The battleship hull, fitted out with everything a battleship needs");
    comp->setTpclRequirementsFunction(
            "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
                "(cons #t \"\") "
                "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist.clear();
    propertylist[1] = "(lambda (design) 300000000)";
    propertylist[2] = "(lambda (design) 4)";
    propertylist[3] = "(lambda (design) 6)";
    propertylist[4] = "(lambda (design) 3)";
    propertylist[5] = "(lambda (design) 1)";
    propertylist[7] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);
  
  
  ObjectManager* obman = game->getObjectManager();

  IGObject* universe = game->getObjectManager()->createNewObject();
  universe->setType(obT_Universe);
  universe->setSize(100000000000ll);
  universe->setName("The Universe");
  universe->setPosition(Vector3d(0ll, 0ll, 0ll));
  universe->setVelocity(Vector3d(0ll, 0ll, 0ll));
  obman->addObject(universe);
  
  //add contained objects
  IGObject *mw_galaxy = game->getObjectManager()->createNewObject();
  mw_galaxy->setSize(10000000000ll);
  mw_galaxy->setType(obT_Galaxy);
  mw_galaxy->setName("Milky Way Galaxy");
  mw_galaxy->setPosition(Vector3d(0ll, -6000ll, 0ll));
  mw_galaxy->setVelocity(Vector3d(0ll, 0ll, 0ll));
  mw_galaxy->addToParent(universe->getID());
  obman->addObject(mw_galaxy);
  
  // star system 1
  IGObject *sol = game->getObjectManager()->createNewObject();
  sol->setSize(60000ll);
  sol->setType(obT_Star_System);
  sol->setName("Sol/Terra System");
  sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
  sol->setVelocity(Vector3d(0ll, 0ll, 0ll));
  sol->addToParent(mw_galaxy->getID());
  obman->addObject(sol);

  // star system 2
  IGObject *ac = game->getObjectManager()->createNewObject();
  ac->setSize(90000ll);
  ac->setType(obT_Star_System);
  ac->setName("Alpha Centauri System");
  ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
  ac->setVelocity(Vector3d(0ll, 0ll, 0ll));
  ac->addToParent(mw_galaxy->getID());
  obman->addObject(ac);
  
  // star system 3
  IGObject *sirius = game->getObjectManager()->createNewObject();
  sirius->setSize(60000ll);
  sirius->setType(obT_Star_System);
  sirius->setName("Sirius System");
  sirius->setPosition(Vector3d(-250000000ll, -4000000000ll, 0ll));
  sirius->setVelocity(Vector3d(0ll, 0ll, 0ll));
  sirius->addToParent(mw_galaxy->getID());
  obman->addObject(sirius);

  
  // now for some planets
  
  IGObject *earth = game->getObjectManager()->createNewObject();
  earth->setSize(2);
  earth->setType(obT_Planet);
  earth->setName("Earth/Terra");
  earth->setPosition(sol->getPosition() + Vector3d(14960ll, 0ll, 0ll));
  earth->addToParent(sol->getID());
  obman->addObject(earth);
  
  IGObject *venus = game->getObjectManager()->createNewObject();
  venus->setSize(2);
  venus->setType(obT_Planet);
  venus->setName("Venus");
  venus->setPosition(sol->getPosition() + Vector3d(0ll, 10800ll, 0ll));
  venus->addToParent(sol->getID());
  obman->addObject(venus);
  
  IGObject *mars = game->getObjectManager()->createNewObject();
  mars->setSize(1);
  mars->setType(obT_Planet);
  mars->setName("Mars");
  mars->setPosition(sol->getPosition() + Vector3d(-22790ll, 0ll, 0ll));
  mars->addToParent(sol->getID());
  obman->addObject(mars);
  
  IGObject *acprime = game->getObjectManager()->createNewObject();
  acprime->setSize(2);
  acprime->setType(obT_Planet);
  acprime->setName("Alpha Centauri Prime");
  acprime->setPosition(ac->getPosition() + Vector3d(-6300ll, 78245ll, 0ll));
  acprime->addToParent(ac->getID());
  obman->addObject(acprime);
  
  IGObject *s1 = game->getObjectManager()->createNewObject();
  s1->setSize(2);
  s1->setType(obT_Planet);
  s1->setName("Sirius 1");
  s1->setPosition(sirius->getPosition() + Vector3d(45925ll, -34262ll, 0ll));
  s1->addToParent(sirius->getID());
  obman->addObject(s1);
  
  std::set<const char*> sys_names(systemNames, systemNames + (sizeof(systemNames) / sizeof(systemNames[0])));
  
  //create random systems
  uint32_t min_systems = atoi(Settings::getSettings()->get("minisec_min_systems").c_str());
  uint32_t max_systems = atoi(Settings::getSettings()->get("minisec_max_systems").c_str());
  uint32_t num_systems;
  if(min_systems == max_systems){
    num_systems = min_systems;
  }else{
    num_systems =  game->getRandom()->getInRange(min_systems ,max_systems);
  }
  if(num_systems > sys_names.size()) 
    num_systems = sys_names.size();
  uint32_t total_planets = atoi(Settings::getSettings()->get("minisec_total_planets").c_str());
  if(total_planets == 0)
    total_planets = 0x0fffffff;
  for (uint32_t counter = 0; counter < num_systems; counter++) {
    createStarSystem( mw_galaxy, total_planets, sys_names);
    if(total_planets <= 0)
      break;
  }
  
    //setup Resources
    ResourceDescription* res = new ResourceDescription();
    res->setNameSingular("Ship part");
    res->setNamePlural("Ship parts");
    res->setUnitSingular("part");
    res->setUnitPlural("parts");
    res->setDescription("Ships parts that can be used to create ships");
    res->setMass(0);
    res->setVolume(0);
    game->getResourceManager()->addResourceDescription(res);
    
    res = new ResourceDescription();
    res->setNameSingular("Home Planet");
    res->setNamePlural("Home Planets");
    res->setUnitSingular("unit");
    res->setUnitPlural("units");
    res->setDescription("The home planet for a race.");
    res->setMass(0);
    res->setVolume(0);
    game->getResourceManager()->addResourceDescription(res);
  
}

void MiniSec::startGame(){
    
    if(Game::getGame()->getResourceManager()->getResourceDescription(1) == NULL){
        Logger::getLogger()->info("Setting up ship part resource that had not been setup");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Ship part");
        res->setNamePlural("Ship parts");
        res->setUnitSingular("part");
        res->setUnitPlural("parts");
        res->setDescription("Ships parts that can be used to create ships");
        res->setMass(0);
        res->setVolume(0);
        Game::getGame()->getResourceManager()->addResourceDescription(res);
    }
    if(Game::getGame()->getResourceManager()->getResourceDescription(2) == NULL){
      Logger::getLogger()->info("Setting up home planet resource that had not been setup");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Home Planet");
        res->setNamePlural("Home Planets");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("The home planet for a race.");
        res->setMass(0);
        res->setVolume(0);
        Game::getGame()->getResourceManager()->addResourceDescription(res);
    }
    
  Game::getGame()->setTurnLength(600);
}

void MiniSec::doOnceATurn(){
  Game* game = Game::getGame();
  std::set<unsigned int> vis = game->getObjectManager()->getAllIds();
    std::set<uint32_t> players = game->getPlayerManager()->getAllIds();
    for(std::set<uint32_t>::iterator itplayer = players.begin(); 
            itplayer != players.end(); ++itplayer){
        game->getPlayerManager()->getPlayer(*itplayer)->setVisibleObjects(vis);
    }
}

bool MiniSec::onAddPlayer(Player* player){
  
  return true;
}

void MiniSec::onPlayerAdded(Player* player){
  Game *game = Game::getGame();

  Logger::getLogger()->debug("MiniSec::onPlayerAdded");

  player->setVisibleObjects(game->getObjectManager()->getAllIds());

  player->addVisibleComponent(1);
  player->addVisibleComponent(2);
  player->addVisibleComponent(3);

  //temporarily add the components as usable to get the designs done
  player->addUsableComponent(1);
  player->addUsableComponent(2);
  player->addUsableComponent(3);

  Design* scout = new Design();
  scout->setCategoryId(1);
  scout->setName("Scout");
  scout->setDescription("Scout ship");
  scout->setOwner(player->getID());
    std::map<unsigned int, unsigned int> cl;
    cl[1] = 1;
  scout->setComponents(cl);
  game->getDesignStore()->addDesign(scout);
  unsigned int scoutid = scout->getDesignId();

    Design* design = new Design();
  design->setCategoryId(1);
  design->setName("Frigate");
  design->setDescription("Frigate ship");
  design->setOwner(player->getID());
  cl.clear();
    cl[2] = 1;
  design->setComponents(cl);
  game->getDesignStore()->addDesign(design);

  design = new Design();
  design->setCategoryId(1);
  design->setName("Battleship");
  design->setDescription("Battleship ship");
  design->setOwner(player->getID());
  cl.clear();
    cl[3] = 1;
  design->setComponents(cl);
  game->getDesignStore()->addDesign(design);

  //remove temporarily added usable components
  player->removeUsableComponent(1);
  player->removeUsableComponent(2);
  player->removeUsableComponent(3);

  const char* name = player->getName().c_str();
  IGObject *star = game->getObjectManager()->createNewObject();
  star->setSize(80000ll);
  star->setType(obT_Star_System);
  char* temp = new char[strlen(name) + 13];
  strncpy(temp, name, strlen(name));
  strncpy(temp + strlen(name), " Star System", 12);
  temp[strlen(name) + 12] = '\0';
  star->setName(temp);
  delete[] temp;
  star->setPosition(Vector3d((long long)(game->getRandom()->getInRange(-5000, 5000) * 10000000),
			     (long long)(game->getRandom()->getInRange(-5000, 5000) * 10000000),
			     /*(long long)(((rand() % 1000) - 500) * 10000000)*/ 0));
  star->setVelocity(Vector3d(0ll, 0ll, 0ll));
  
  star->addToParent(1);
  game->getObjectManager()->addObject(star);
  
  IGObject *planet = game->getObjectManager()->createNewObject();
  planet->setSize(2);
  planet->setType(obT_Planet);
  temp = new char[strlen(name) + 8];
  strncpy(temp, name, strlen(name));
  strncpy(temp + strlen(name), " Planet", 7);
  temp[strlen(name) + 7] = '\0';
  planet->setName(temp);
  delete[] temp;
  ((OwnedObject*)(planet->getObjectData()))->setOwner(player->getID());
  ((Planet*)(planet->getObjectData()))->addResource(2, 1);
  planet->setPosition(star->getPosition() + Vector3d((long long)(game->getRandom()->getInRange(-5000, 5000)* 10),
						     (long long)(game->getRandom()->getInRange(-5000, 5000) * 10),
						     /*(long long)((rand() % 10000) - 5000)*/ 0));
  planet->setVelocity(Vector3d(0LL, 0ll, 0ll));
  
  planet->addToParent(star->getID());
  game->getObjectManager()->addObject(planet);
  
  IGObject *fleet = game->getObjectManager()->createNewObject();
  fleet->setSize(2);
  fleet->setType(obT_Fleet);
  temp = new char[strlen(name) + 13];
  strncpy(temp, name, strlen(name));
  strncpy(temp + strlen(name), " First Fleet", 12);
  temp[strlen(name) + 12] = '\0';
  fleet->setName(temp);
  delete[] temp;
  ((OwnedObject*)(fleet->getObjectData()))->setOwner(player->getID());
  fleet->setPosition(star->getPosition() + Vector3d((long long)(game->getRandom()->getInRange(-5000, 5000) * 10),
						    (long long)(game->getRandom()->getInRange(-5000, 5000) * 10),
						    /*(long long)((rand() % 10000) - 5000)*/ 0));
  ((Fleet*)(fleet->getObjectData()))->addShips(scoutid, 2);
    scout->addUnderConstruction(2);
    scout->addComplete(2);
    game->getDesignStore()->designCountsUpdated(scout);
  fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
  
  fleet->addToParent(star->getID());
  game->getObjectManager()->addObject(fleet);

    game->getPlayerManager()->updatePlayer(player->getID());
}

// Create a random star system
IGObject* MiniSec::createStarSystem( IGObject* mw_galaxy, uint32_t& max_planets, std::set<const char*>& systemnames)
{
    Logger::getLogger()->debug( "Entering MiniSec::createStarSystem");
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    IGObject*      star = game->getObjectManager()->createNewObject();
    unsigned int   nplanets = 0;
    std::ostringstream     formatter;

    // Create a variable number of planets for each star system
    uint maxplanets = atoi(Settings::getSettings()->get("minisec_max_planets").c_str());
    uint minplanets = atoi(Settings::getSettings()->get("minisec_min_planets").c_str());
    if(minplanets == maxplanets){
      nplanets = minplanets;
    }else{
      nplanets = game->getRandom()->getInRange(minplanets, maxplanets);
    }
    if(max_planets < nplanets)
      nplanets = max_planets;

    star->setSize(nplanets * 60000ll);
    star->setType( obT_Star_System);
    unsigned int   thx = game->getRandom()->getInRange(0U, systemnames.size() - 1);
    std::set<const char*>::iterator name = systemnames.begin();
    advance(name, thx);
    if(name == systemnames.end())
      name = systemnames.begin();
    star->setName(*name);
    systemnames.erase(name);
    star->setPosition( Vector3d( game->getRandom()->getInRange(0, 8000) * 1000000ll - 4000000000ll,
                                 game->getRandom()->getInRange(0, 8000) * 1000000ll - 4000000000ll,
                                 0ll));
    star->setVelocity( Vector3d( 0ll, 0ll, 0ll));
    star->addToParent( mw_galaxy->getID());
    obman->addObject( star);

    for(uint i = 1; i <= nplanets; i++){
        IGObject*  planet = game->getObjectManager()->createNewObject();
        formatter.str("");
        formatter << star->getName() << " " << i;

        planet->setSize( 2);
        planet->setType( obT_Planet);
        planet->setName( formatter.str().c_str());
        planet->setPosition( star->getPosition() + Vector3d( i * 40000ll,
                                                             i * -35000ll,
                                                             0ll));
        planet->addToParent( star->getID());
        obman->addObject( planet);
        max_planets--;
    }

    Logger::getLogger()->debug( "Exiting MiniSec::createStarSystem");
    return star;
}
