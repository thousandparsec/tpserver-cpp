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

#include <tpserver/player.h>
#include <tpserver/playermanager.h>
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

#include "nop.h"
#include "emptyobject.h"
#include "planet.h"
#include "universe.h"

#include "rfts.h"

// hacky define :p
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__FUNCTION__))

extern "C" {
  #define tp_init librfts_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setRuleset(new RFTS_::Rfts());
  }
}

namespace RFTS_ {

using std::string;
using std::map;

Rfts::Rfts() {

}

Rfts::~Rfts() {

}

std::string Rfts::getName() {
   return "TP RFTS";
}

std::string Rfts::getVersion() {
   return "0.0";
}

void Rfts::initGame() {
   DEBUG_FN_PRINT();
   
   // opt. set custom turn process.

   setObjectTypes();

   setOrderTypes();
}

void Rfts::setObjectTypes() const {

   DEBUG_FN_PRINT();

   ObjectDataManager* obdm = Game::getGame()->getObjectDataManager();
   EmptyObject *eo;

   obdm->addNewObjectType(new Universe);

   // added for tp03
   eo = new EmptyObject();
   eo->setTypeName("Galaxy");
   eo->setTypeDescription("Galaxy");
   obdm->addNewObjectType(eo);

   eo = new EmptyObject();
   eo->setTypeName("Star System"); 
   eo->setTypeDescription("A system of stars!");
   obdm->addNewObjectType(eo);

   obdm->addNewObjectType(new Planet);
   //obdm->addNewObjectType(new Fleet);
}

void Rfts::setOrderTypes() const {
   OrderManager* orm = Game::getGame()->getOrderManager();

   orm->addOrderType(new Nop());
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
   prop->setCategoryId(1);
   prop->setRank(0);
   prop->setName("Speed");
   prop->setDisplayName("Speed");
   prop->setDescription("The number of units the ship can move each turn");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000)) \" speedy units\")) ) )");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);
   
   // attack
   prop = new Property();
   prop->setCategoryId(1);
   prop->setRank(0);
   prop->setName("Attack");
   prop->setDisplayName("Attack");
   prop->setDescription("The offensive strength of a ship");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);

   // armour
   prop = new Property();
   prop->setCategoryId(1);
   prop->setRank(0);
   prop->setName("Armour");
   prop->setDisplayName("Armour");
   prop->setDescription("The defensive strength of a ship");
   prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
   prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
   ds->addProperty(prop);

   // colonise
   prop = new Property();
   prop->setCategoryId(1);
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

Component* Rfts::createEngineComponent(char techLevel) {

   Component* engine = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   engine->setCategoryId(1); // check
   engine->setName( string("Engine") + techLevel);
   engine->setDescription( "A ship engine, required if you want your ship to move!");
   engine->setTpclRequirementsFunction(
      "(lambda (design) "
      /*"(if (= (designType._num-components design) 1) "
      "(cons #t \"\") "*/
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ds->getPropertyByName("Speed")] = string("(lambda (design) (* 100 ") +  techLevel + string("))");
   engine->setPropertyList(propList);

   return engine;
}

Component* Rfts::createBattleComponent(char techLevel) {
   Component *battle = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   battle->setCategoryId(1); // check
   battle->setName( string("Battle") + techLevel);
   battle->setDescription( "Guns and armour for a ship");
   battle->setTpclRequirementsFunction(
      "(lambda (design) "
      /*"(if (= (designType._num-components design) 1) "
      "(cons #t \"\") "*/
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ ds->getPropertyByName("Attack") ] = string("(lambda (design) (* 5") + techLevel + string("))");
   propList[ ds->getPropertyByName("Armour") ] = string("(lambda (design) (* 5") + techLevel + string("))");
   battle->setPropertyList(propList);
   return battle;
}


Component* Rfts::createTransportComponent() {
   Component *trans = new Component();
   map<unsigned int, string> propList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   trans->setCategoryId(1); // check
   trans->setName( "Transport");
   trans->setDescription( "A colonist transport bay");
   trans->setTpclRequirementsFunction(
      "(lambda (design) "
      /*"(if (= (designType._num-components design) 1) "
      "(cons #t \"\") "*/
      "(cons #f \"This is a complete component, nothing else can be included\")))");
   propList[ ds->getPropertyByName("Colonise") ] = "(lambda (design) 1)";   
   trans->setPropertyList(propList);
   return trans;
}

void Rfts::createUniverse() const {
   DEBUG_FN_PRINT();

   ObjectManager *objman = Game::getGame()->getObjectManager();

   uint32_t uniType = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Universe");
   IGObject *universe = objman->createNewObject();

   universe->setType(uniType);
   universe->setName("The Universe");
   EmptyObject* uniData = static_cast<EmptyObject*>(universe->getObjectData());
   uniData->setPosition(Vector3d(0ll, 0ll, 0ll));
   objman->addObject(universe);   
   
   createStarSystems(universe);
}

void Rfts::createStarSystems(IGObject *universe) const {
   DEBUG_FN_PRINT();
   // todo (make all the systems... and functions for each)
   // just create a single test system for now

   Game *game = Game::getGame();
   ObjectManager *objman = game->getObjectManager();
   //ResourceManager* resman = game->getResourceManager();
   IGObject *starSys = game->getObjectManager()->createNewObject();
   IGObject *planet = game->getObjectManager()->createNewObject();   
   //std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
   
   uint32_t ssType = game->getObjectDataManager()->getObjectTypeByName("Star System");
   uint32_t planetType = game->getObjectDataManager()->getObjectTypeByName("Planet");

   starSys->setType(ssType);
   starSys->setName("Star System1");
   EmptyObject* starSysData = static_cast<EmptyObject*>(starSys->getObjectData());
   starSysData->setPosition(Vector3d(3000ll, 200ll, 0ll));
   
   starSys->addToParent(universe->getID());
   objman->addObject(starSys);
   
   planet->setType(planetType);
   planet->setName("Planet1");
   Planet* planetData = static_cast<Planet*>(planet->getObjectData());
   planetData->setSize(2);
   planetData->setPosition(starSysData->getPosition() + Vector3d(14960ll, 0ll, 0ll));
   // set up resources
   //ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
   //((Planet*)(planet->getObjectData()))->setResources(ress);
   
   OrderQueue *planetOrders = new OrderQueue();
   planetOrders->setQueueId(planet->getID());
   planetOrders->addOwner(0); // check
   game->getOrderManager()->addOrderQueue(planetOrders);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planetData->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(planetOrders->getQueueId());
   planetData->setDefaultOrderTypes();
  
   planet->addToParent(starSys->getID());
   objman->addObject(planet);
}

void Rfts::createResources() const {
   //todo - something
}

void Rfts::startGame() {
	DEBUG_FN_PRINT();
}

Design* Rfts::createMarkDesign(Player *owner, char level) const {
   Design *mark = new Design();
   DesignStore *ds = Game::getGame()->getDesignStore();
   map<unsigned int, unsigned int> componentList;

   string name = "Mark " + level;

   mark->setCategoryId(1); // check
   mark->setName( name ); // add level
   mark->setDescription( name + string(" battle ship") );
   mark->setOwner( owner->getID() );
   componentList[ ds->getComponentByName(string("Engine") + level) ] = 1;
   componentList[ ds->getComponentByName(string("Battle") + level) ] = 1;
   mark->setComponents(componentList);

   return mark;
}


Design* Rfts::createScoutDesign(Player *owner) const {
   Design* scout = new Design();
   map<unsigned int, unsigned int> componentList;

   DesignStore *ds = Game::getGame()->getDesignStore();

   scout->setCategoryId(1); // check
   scout->setName( "Scout");
   scout->setDescription("Scout ship");
   scout->setOwner( owner->getID());
   componentList[ ds->getComponentByName("Engine1") ] = 1;
   scout->setComponents(componentList);

    return scout;
}

bool Rfts::onAddPlayer(Player *player) {
   DEBUG_FN_PRINT();
   if(Game::getGame()->getPlayerManager()->getNumPlayers() < MAX_PLAYERS)
      return true;
   return false;
}
void Rfts::onPlayerAdded(Player *player) {
   DEBUG_FN_PRINT();

   // set designs (?)
   // ^^ apply to player
}

}
