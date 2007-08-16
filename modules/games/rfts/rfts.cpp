/*  RFTS rulesset
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>

#include <tpserver/property.h>
#include <tpserver/component.h>
#include <tpserver/design.h>
#include <tpserver/category.h>
#include <tpserver/designstore.h>

#include <tpserver/object.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/prng.h>
#include <tpserver/settings.h>

#include "nop.h"
#include "buildfleet.h"
#include "move.h"
#include "productionorder.h"
#include "splitfleet.h"
#include "mergefleet.h"
#include "renamefleet.h"
#include "colonise.h"
#include "bombard.h"

#include "staticobject.h"
#include "planet.h"
#include "universe.h"
#include "fleet.h"

#include "rftsturn.h"
#include "productioninfo.h"
#include "playerinfo.h"

#include "rfts.h"

// hacky define :p
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__PRETTY_FUNCTION__))

extern "C" {
  #define tp_init librfts_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setRuleset(new RFTS_::Rfts());
  }
}

namespace RFTS_ {

using std::string;
using std::map;
using std::set;
using std::vector;
using std::advance;

Rfts::Rfts() {

}

Rfts::~Rfts() {

}

std::string Rfts::getName() {
   return "TP RFTS";
}

std::string Rfts::getVersion() {
   return "0.1";
}

const ProductionInfo& Rfts::getProductionInfo() {
   static ProductionInfo prodInfo = ProductionInfo();
   return prodInfo;
}

void Rfts::initGame() {
   DEBUG_FN_PRINT();
   
   Game::getGame()->setTurnProcess(new RftsTurn());

   setObjectTypes();

   setOrderTypes();
}

void Rfts::setObjectTypes() const {

   DEBUG_FN_PRINT();

   ObjectDataManager* obdm = Game::getGame()->getObjectDataManager();
   StaticObject *eo;

   obdm->addNewObjectType(new Universe);

   // galaxy added for tp03
   eo = new StaticObject();
   eo->setTypeName("Galaxy");
   eo->setTypeDescription("Galaxy");
   obdm->addNewObjectType(eo);

   eo = new StaticObject();
   eo->setTypeName("Star System"); 
   eo->setTypeDescription("A system of stars!");
   obdm->addNewObjectType(eo);

   obdm->addNewObjectType(new Planet);
   obdm->addNewObjectType(new Fleet);
}

void Rfts::setOrderTypes() const {
   OrderManager* orm = Game::getGame()->getOrderManager();

   orm->addOrderType(new Nop());
   orm->addOrderType(new BuildFleet());
   orm->addOrderType(new Move());
   orm->addOrderType(new ProductionOrder());
   orm->addOrderType(new SplitFleet());
   orm->addOrderType(new MergeFleet());
   orm->addOrderType(new RenameFleet());
   orm->addOrderType(new Colonise());
   orm->addOrderType(new Bombard());
}

void Rfts::createGame() {
   DEBUG_FN_PRINT();
   
   Game *game = game->getGame();
   
   // create general category
   Category* cat = new Category();
   cat->setName("Ships");
   cat->setDescription("The ship design & component category");
   game->getDesignStore()->addCategory(cat);

   createProperties();

   createComponents();

   createResources();
   
   // set up universe (universe -> star sys -> planets)
   createUniverse(); // wow that looks like a powerful function!
}

void Rfts::createProperties() {
   Property* prop = new Property();
   DesignStore *ds = Game::getGame()->getDesignStore();

   // speed   

   prop->addCategoryId(ds->getCategoryByName("Ships"));
   prop->setRank(0);
   prop->setName("Speed");
   prop->setDisplayName("Speed");
   prop->setDescription("The number of units the ship can move each turn");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000)) \" speedy units\")) ) )");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);
   
   // attack
   prop = new Property();
   prop->addCategoryId(ds->getCategoryByName("Ships"));
   prop->setRank(0);
   prop->setName("Attack");
   prop->setDisplayName("Attack");
   prop->setDescription("The offensive strength of a ship");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);

   // armour
   prop = new Property();
   prop->addCategoryId(ds->getCategoryByName("Ships"));
   prop->setRank(0);
   prop->setName("Armour");
   prop->setDisplayName("Armour");
   prop->setDescription("The defensive strength of a ship");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);

   // colonise
   prop = new Property();
   prop->addCategoryId(ds->getCategoryByName("Ships"));
   prop->setName("Colonise");
   prop->setDisplayName("Can Colonise");
   prop->setDescription("The ship colonise planets");
   prop->setRank(0);
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (if (= n 1) \"Yes\" \"No\")) ) )");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);
}

void Rfts::createComponents() {
   DesignStore *ds = Game::getGame()->getDesignStore();  
   
   // movement
   ds->addComponent(createEngineComponent('1'));
   ds->addComponent(createEngineComponent('2'));
   ds->addComponent(createEngineComponent('3'));
   ds->addComponent(createEngineComponent('4'));

   // attack
   ds->addComponent(createBattleComponent('1'));
   ds->addComponent(createBattleComponent('2'));
   ds->addComponent(createBattleComponent('3'));
   ds->addComponent(createBattleComponent('4'));

   // colonise
   ds->addComponent(createTransportComponent());
}

void Rfts::createUniverse() {
   DEBUG_FN_PRINT();

   ObjectManager *objman = Game::getGame()->getObjectManager();

   uint32_t uniType = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Universe");
   IGObject *universe = objman->createNewObject();

   universe->setType(uniType);
   universe->setName("The Universe");
   StaticObject* uniData = static_cast<StaticObject*>(universe->getObjectData());
   uniData->setPosition(Vector3d(0ll, 0ll, 0ll));
   uniData->setSize(123456789012ll);
   objman->addObject(universe);   

   vector<string> planetNames;
   planetNames.push_back(string("Castor Prime"));
   createStarSystem(*universe, "Castor", Vector3d(100000000, 50000000, 0), planetNames);

   planetNames.clear();
   planetNames.push_back("Dipha Prime");
   createStarSystem(*universe, "Diphda", Vector3d(100000000, 500000000, 0), planetNames);

   planetNames.clear();
   planetNames.push_back("Saiph Prime");
   createStarSystem(*universe, "Saiph", Vector3d(600000000, 250000000, 0), planetNames);
}

IGObject* Rfts::createStarSystem(IGObject& universe, const string& name,
                  const Vector3d& location, const vector<string>& planetNames)
{
   Game *game = Game::getGame();
   
   IGObject *starSys = game->getObjectManager()->createNewObject();

   starSys->setType(game->getObjectDataManager()->getObjectTypeByName("Star System"));
   starSys->setName(name);
   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSys->getObjectData());
   starSysData->setPosition(location);
   
   starSys->addToParent(universe.getID());
   game->getObjectManager()->addObject(starSys);

   Random* rand = game->getRandom();

   for(vector<string>::const_iterator i = planetNames.begin(); i != planetNames.end(); ++i)
      createPlanet(*starSys, *i, starSysData->getPosition() +
         Vector3d(rand->getInRange(100000,3000000), rand->getInRange(100000,3000000),
                   rand->getInRange(1000,30000)));

   return starSys;
}

IGObject* Rfts::createPlanet(IGObject& parentStarSys, const string& name, const Vector3d& location) {

   Game *game = Game::getGame();

   IGObject *planet = game->getObjectManager()->createNewObject();

   planet->setType(game->getObjectDataManager()->getObjectTypeByName("Planet"));
   planet->setName(name);
   Planet* planetData = static_cast<Planet*>(planet->getObjectData());
   planetData->setSize(3);
   planetData->setPosition(location);
   planetData->setDefaultResources();
   
   OrderQueue *planetOrders = new OrderQueue();
   planetOrders->setObjectId(planet->getID());
   planetOrders->addOwner(0);
   game->getOrderManager()->addOrderQueue(planetOrders);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>
                                       (planetData->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(planetOrders->getQueueId());
   planetData->setOrderTypes();
  
   planet->addToParent(parentStarSys.getID());
   game->getObjectManager()->addObject(planet);

   return planet;
}

void Rfts::createResources() const {
   ResourceManager* resMan = Game::getGame()->getResourceManager();
   
   ResourceDescription* res = new ResourceDescription();
   res->setNameSingular("Resource Point");
   res->setNamePlural("Resource Points");
   res->setUnitSingular("point");
   res->setUnitPlural("points");
   res->setDescription("Resource points");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Industry");
   res->setNamePlural("Industry");
   res->setUnitSingular("units");
   res->setUnitPlural("units");
   res->setDescription("Industrial strength");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Population");
   res->setNamePlural("Population");
   res->setUnitSingular("unit");
   res->setUnitPlural("units");
   res->setDescription("Population of the planet");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Social Environment");
   res->setNamePlural("Social Environment");
   res->setUnitSingular("point");
   res->setUnitPlural("points");
   res->setDescription("Social Env. describes the health of the population. (Influences Population)");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Planetary Environment");
   res->setNamePlural("Planetary Environment");
   res->setUnitSingular("point");
   res->setUnitPlural("points");
   res->setDescription("Planetary Env. describes the health of the planet. (Influences Social Env.)");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Population Maintenance");
   res->setNamePlural("Population Maintenance");
   res->setUnitSingular("point");
   res->setUnitPlural("points");
   res->setDescription("Population Maintenance is the cost required to maintain the current population.\
                         A 1:1 ratio with population is required for healthy populations.");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);

   res = new ResourceDescription();
   res->setNameSingular("Colonist");
   res->setNamePlural("Colonists");
   res->setUnitSingular("unit");
   res->setUnitPlural("units");
   res->setDescription("Population available for colonisation");
   res->setMass(0);
   res->setVolume(0);
   resMan->addResourceDescription(res);
}

void Rfts::startGame() {
   DEBUG_FN_PRINT();
   
   Settings* settings = Settings::getSettings();
   if(settings->get("turn_length_over_threshold") == ""){
      settings->set("turn_length_over_threshold", "170");
      settings->set("turn_player_threshold", "0");
      settings->set("turn_length_under_threshold", "170");
   }
}

bool Rfts::onAddPlayer(Player *player) {
   DEBUG_FN_PRINT();
   if(Game::getGame()->getPlayerManager()->getNumPlayers() < MAX_PLAYERS)
      return true;
   return false;
}
void Rfts::onPlayerAdded(Player *player) {
   DEBUG_FN_PRINT();

   Game *game = Game::getGame();
   ObjectManager *om = Game::getGame()->getObjectManager();

   PlayerView* playerview = player->getPlayerView();

   for(uint32_t i = 1; i <= game->getDesignStore()->getMaxComponentId(); ++i){
      playerview->addUsableComponent(i);
   }

   // test : set the 2nd object - a planet - to be owned by the player
   IGObject *homePlanet = choosePlayerPlanet();
   Planet* pData = dynamic_cast<Planet*>(homePlanet->getObjectData());
   pData->setOwner(player->getID());

   Logger::getLogger()->debug("Making player's fleet");
   
   IGObject* fleet = createEmptyFleet( player, om->getObject(homePlanet->getParent()),
                                        player->getName() + "'s fleet");

   // give 'em a scout to start
   Design* scout = createScoutDesign(player);
   
   dynamic_cast<Fleet*>(fleet->getObjectData())->addShips( scout->getDesignId(), 1);
   
   game->getDesignStore()->designCountsUpdated(scout);

   // start them out with access to mark1
   game->getDesignStore()->designCountsUpdated(createMarkDesign(player, '1'));

   // and a transport (save the transport id for easy searching later)
   Design *trans = createTransportDesign(player);
   PlayerInfo::getPlayerInfo(player->getID()).setTransportId(trans->getDesignId());
   game->getDesignStore()->designCountsUpdated(trans);

   game->getObjectManager()->addObject(fleet);

   Logger::getLogger()->debug( "done making fleet");

   // set visible objects
   set<uint32_t> gameObjects = om->getAllIds();
   set<uint32_t> ownedObjects;
   findOwnedObjects(player->getID(), gameObjects, ownedObjects);
   setVisibleObjects(player, ownedObjects);

   Game::getGame()->getPlayerManager()->updatePlayer(player->getID());
}


// make sure to start the player in a non-occupied area
IGObject* Rfts::choosePlayerPlanet() const {
   DEBUG_FN_PRINT();

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();
   Random *rand = game->getRandom();

   IGObject *universe = om->getObject(0);
   set<uint32_t> starSystems = universe->getContainedObjects();

   IGObject *homePlanet = NULL;
   unsigned searchedSystems = 0;

   while(homePlanet == NULL && searchedSystems < starSystems.size() * 3./4)
   {
      // pick rand Star System to search
      set<uint32_t>::iterator starSysI = starSystems.begin();
      advance(starSysI, rand->getInRange(static_cast<uint32_t>(0), starSystems.size()-1));
      IGObject *starSys = om->getObject(*starSysI);
      
      searchedSystems++;
      
      set<uint32_t> planets = starSys->getContainedObjects(); // (might not -actually- be planets)
      unsigned starSysClear = 0;
      for(set<uint32_t>::iterator i = planets.begin(); i != planets.end(); i++)
      {
         OwnedObject *owned = dynamic_cast<OwnedObject*>(om->getObject(*i)->getObjectData());
         if(owned->getOwner() == 0)
            starSysClear++;
         
      }

      if(starSysClear == planets.size())
      {
         set<uint32_t>::iterator i = planets.begin();
         advance(i, rand->getInRange(static_cast<uint32_t>(0), planets.size()-1));
         homePlanet = om->getObject(*i); // must be a planet because it's owner-less
      }
   }

   assert(homePlanet != NULL); // possible, will write a last-ditch effort later maybe
   
   return homePlanet;
}

// helper functions

Component* createEngineComponent(char techLevel) {

   Component* engine = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   engine->addCategoryId(ds->getCategoryByName("Ships"));
   engine->setName( string("Engine") + techLevel);
   engine->setDescription( "A ship engine, required if you want your ship to move!");
   engine->setTpclRequirementsFunction(
      "(lambda (design) "
      "(if (< (designType._num-components design) 3) "
      "(cons #t \"\") "
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ds->getPropertyByName("Speed")] = string("(lambda (design) (* 100 ") +  techLevel + string("))");
   engine->setPropertyList(propList);

   return engine;
}

Component* createBattleComponent(char techLevel) {
   Component *battle = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   battle->addCategoryId(ds->getCategoryByName("Ships"));
   battle->setName( string("Battle") + techLevel);
   battle->setDescription( "Guns and armour for a ship");
   battle->setTpclRequirementsFunction(
      "(lambda (design) "
      "(if (< (designType._num-components design) 3) "
      "(cons #t \"\") "
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ ds->getPropertyByName("Attack") ] = string("(lambda (design) (* 5 ") + techLevel + string("))");
   propList[ ds->getPropertyByName("Armour") ] = string("(lambda (design) (* 5 ") + techLevel + string("))");
   battle->setPropertyList(propList);
   return battle;
}


Component* createTransportComponent() {
   Component *trans = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   trans->addCategoryId(ds->getCategoryByName("Ships"));
   trans->setName( "Transport");
   trans->setDescription( "A colonist transport bay");
   trans->setTpclRequirementsFunction(
      "(lambda (design) "
      "(if (< (designType._num-components design) 3) "
      "(cons #t \"\") "
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ ds->getPropertyByName("Colonise") ] = "(lambda (design) 1)";
   trans->setPropertyList(propList);
   return trans;
}

Design* createMarkDesign(Player *owner, char level) {
   Design *mark = new Design();
   DesignStore *ds = Game::getGame()->getDesignStore();
   map<unsigned int, unsigned int> componentList;

   string name = string("Mark") + level;

   mark->setCategoryId(ds->getCategoryByName("Ships"));
   mark->setName( name );
   mark->setDescription( name + string(" battle ship") );
   mark->setOwner( owner->getID() );
   componentList[ ds->getComponentByName(string("Engine") + level) ] = 1;
   componentList[ ds->getComponentByName(string("Battle") + level) ] = 1;
   mark->setComponents(componentList);

   ds->addDesign(mark);

   return mark;
}


Design* createScoutDesign(Player *owner) {
   Design* scout = new Design();
   map<unsigned int, unsigned int> componentList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   scout->setCategoryId(ds->getCategoryByName("Ships"));
   scout->setName( "Scout");
   scout->setDescription("Scout ship");
   scout->setOwner( owner->getID());
   componentList[ ds->getComponentByName("Engine1") ] = 1;
   scout->setComponents(componentList);

   ds->addDesign(scout);

   return scout;
}

Design* createTransportDesign(Player *owner) {
   Design* trans = new Design();
   map<unsigned int, unsigned int> componentList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   trans->setCategoryId(ds->getCategoryByName("Ships"));
   trans->setName("Transport");
   trans->setDescription("Transport ship");
   trans->setOwner( owner->getID());
   componentList[ ds->getComponentByName("Engine1") ] = 1;
   componentList[ ds->getComponentByName("Transport") ] = 1;
   trans->setComponents(componentList);

   ds->addDesign(trans);

   return trans;
}

}
