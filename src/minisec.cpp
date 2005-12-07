/*  MiniSec ruleset
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "game.h"
#include "object.h"
#include "objectmanager.h"
#include "ownedobject.h"
#include "universe.h"
#include "emptyobject.h"
#include "planet.h"
#include "fleet.h"
#include "objectdatamanager.h"
#include "player.h"
#include "rspcombat.h"
#include "designstore.h"
#include "ordermanager.h"
#include "nop.h"
#include "move.h"
#include "build.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"
#include "property.h"
#include "component.h"
#include "design.h"
#include "category.h"
#include "logging.h"

#ifdef HAVE_LIBMYSQL
#include "mysqlpersistence.h"
#include "mysqluniverse.h"
#include "mysqlemptyobject.h"
#include "mysqlplanet.h"
#include "mysqlfleet.h"
#include "mysqlordernop.h"
#include "mysqlordermove.h"
#include "mysqlorderbuild.h"
#include "mysqlordercolonise.h"
#include "mysqlordersplitfleet.h"
#include "mysqlordermergefleet.h"
#endif

#include "minisec.h"

MiniSec::MiniSec(){

}

MiniSec::~MiniSec(){

}

void MiniSec::initGame(){
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
  prop->setName("Amour");
    prop->setDisplayName("Amour");
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
  propertylist[1] = "(lambda (design) 300000000)";
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
  propertylist[1] = "(lambda (design) 100000000)";
  propertylist[2] = "(lambda (design) 4)";
  propertylist[3] = "(lambda (design) 6)";
  propertylist[4] = "(lambda (design) 3)";
  propertylist[5] = "(lambda (design) 1)";
  propertylist[7] = "(lambda (design) 1)";
  comp->setPropertyList(propertylist);
  ds->addComponent(comp);

  

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

#ifdef HAVE_LIBMYSQL
    if(database != NULL){
        MysqlOrderNop* nop = new MysqlOrderNop();
        nop->setType(0);
        database->addOrderType(nop);
        MysqlOrderMove* move = new MysqlOrderMove();
        move->setType(1);
        database->addOrderType(move);
        MysqlOrderBuild* build = new MysqlOrderBuild();
        build->setType(2);
        database->addOrderType(build);
        MysqlOrderColonise* colonise = new MysqlOrderColonise();
        colonise->setType(3);
        database->addOrderType(colonise);
        MysqlOrderSplitFleet* sf = new MysqlOrderSplitFleet();
        sf->setType(4);
        database->addOrderType(sf);
        MysqlOrderMergeFleet* mf = new MysqlOrderMergeFleet();
        mf->setType(5);
        database->addOrderType(mf);
    }
#endif
}

void MiniSec::createGame(){
  Game* game = Game::getGame();
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
  sol->setSize(1400000ll);
  sol->setType(obT_Star_System);
  sol->setName("Sol/Terra System");
  sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
  sol->setVelocity(Vector3d(0ll, 0ll, 0ll));
  sol->addToParent(mw_galaxy->getID());
  obman->addObject(sol);

  // star system 2
  IGObject *ac = game->getObjectManager()->createNewObject();
  ac->setSize(800000ll);
  ac->setType(obT_Star_System);
  ac->setName("Alpha Centauri System");
  ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
  ac->setVelocity(Vector3d(0ll, 0ll, 0ll));
  ac->addToParent(mw_galaxy->getID());
  obman->addObject(ac);
  
  // star system 3
  IGObject *sirius = game->getObjectManager()->createNewObject();
  sirius->setSize(2000000ll);
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
  
  game->setCombatStrategy(new RSPCombat());
  
}

void MiniSec::startGame(){
  Game::getGame()->setTurnLength(600);
}

void MiniSec::doOnceATurn(){
  Game* game = Game::getGame();
  std::set<unsigned int> vis = game->getObjectManager()->getAllIds();
  std::set<unsigned int> players = game->getPlayerIds();
  for(std::set<unsigned int>::iterator itplayer = players.begin(); 
      itplayer != players.end(); ++itplayer){
    game->getPlayer(*itplayer)->setVisibleObjects(vis);
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

  Design* design = new Design();
  design->setCategoryId(1);
  design->setName("Scout");
  design->setDescription("Scout ship");
  design->setOwner(player->getID());
    std::map<unsigned int, unsigned int> cl;
    cl[1] = 1;
  design->setComponents(cl);
  game->getDesignStore()->addDesign(design);
  unsigned int scoutid = design->getDesignId();

  design = new Design();
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

  char* name = player->getName();
  IGObject *star = game->getObjectManager()->createNewObject();
  star->setSize(2000000ll);
  star->setType(obT_Star_System);
  char* temp = new char[strlen(name) + 13];
  strncpy(temp, name, strlen(name));
  strncpy(temp + strlen(name), " Star System", 12);
  temp[strlen(name) + 12] = '\0';
  star->setName(temp);
  delete[] temp;
  star->setPosition(Vector3d((long long)(((rand() % 1000) - 500) * 10000000),
			     (long long)(((rand() % 1000) - 500) * 10000000),
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
  planet->setPosition(star->getPosition() + Vector3d((long long)((rand() % 10000) - 5000),
						     (long long)((rand() % 10000) - 5000),
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
  fleet->setPosition(star->getPosition() + Vector3d((long long)((rand() % 10000) - 5000),
						    (long long)((rand() % 10000) - 5000),
						    /*(long long)((rand() % 10000) - 5000)*/ 0));
  ((Fleet*)(fleet->getObjectData()))->addShips(scoutid, 2);
  fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
  
  fleet->addToParent(star->getID());
  game->getObjectManager()->addObject(fleet);

}
