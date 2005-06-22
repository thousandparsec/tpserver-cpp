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

#include "game.h"
#include "object.h"
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

#include "minisec.h"

MiniSec::MiniSec(){

}

MiniSec::~MiniSec(){

}

void MiniSec::initGame(){
  Game* game = Game::getGame();
  
  DesignStore *ds = new DesignStore();
  ds->setName("Ships");
  game->addDesignStore(ds);
  assert(ds->getCategoryId() == 1);

  Property* prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("Speed");
  prop->setDescription("The number of units the ship can move each turn");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000000)) \" mega-units\")) ) )");
  ds->addProperty(prop);

  prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("BuildTime");
  prop->setDescription("The number of turns to build the ship");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" turns\")) ) )");
  ds->addProperty(prop);
  
  prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("Amour");
  prop->setDescription("The amount of amour on the ship");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
  ds->addProperty(prop);

  prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("WeaponWin");
  prop->setDescription("The number of HP to do to the fired at ship when RSP wins");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
  ds->addProperty(prop);

  prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("WeaponDraw");
  prop->setDescription("The number of HP to do to the fired at ship when RSP draws");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
  ds->addProperty(prop);

  prop = new Property();
  prop->setCategoryId(1);
  prop->setRank(0);
  prop->setName("Colonise");
  prop->setDescription("Can the ship colonise planets");
  prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (if (= n 1) \"Yes\" \"No\")) ) )");
  ds->addProperty(prop);

  std::map<unsigned int, std::string> propertylist;

  Component* comp = new Component();
  comp->setCategoryId(1);
  comp->setName("ScoutHull");
  comp->setDescription("The scout hull, fitted out with everything a scout needs");
  comp->setTpclAddFunction("(lambda (design) (if (> design.Speed 0) (cons #t \"\") (cons #f \"This is a complete component, nothing else can be included\")))");
  propertylist[1] = "(lambda (design) 300000000)";
  propertylist[2] = "(lambda (design) 1)";
  propertylist[3] = "(lambda (design) 2)";
  propertylist[4] = "(lambda (design) 0)";
  propertylist[5] = "(lambda (design) 0)";
  comp->setPropertyList(propertylist);
  ds->addComponent(comp);

  comp = new Component();
  comp->setCategoryId(1);
  comp->setName("FrigateHull");
  comp->setDescription("The frigate hull, fitted out with everything a frigate needs");
  comp->setTpclAddFunction("(lambda (design) (if (> designType.Speed design 0) (cons #t \"\") (cons #f \"This is a complete component, nothing else can be included\")))");
  propertylist.clear();
  propertylist[1] = "(lambda (design) 200000000)";
  propertylist[2] = "(lambda (design) 2)";
  propertylist[3] = "(lambda (design) 4)";
  propertylist[4] = "(lambda (design) 2)";
  propertylist[5] = "(lambda (design) 0)";
  propertylist[6] = "(lambda (design) 1)";
  comp->setPropertyList(propertylist);
  ds->addComponent(comp);

  comp = new Component();
  comp->setCategoryId(1);
  comp->setName("BattleshipHull");
  comp->setDescription("The battleship hull, fitted out with everything a battleship needs");
  comp->setTpclAddFunction("(lambda (design) (if (> designType.Speed design 0) (cons #t \"\") (cons #f \"This is a complete component, nothing else can be included\")))");
  propertylist.clear();
  propertylist[1] = "(lambda (design) 100000000)";
  propertylist[2] = "(lambda (design) 4)";
  propertylist[3] = "(lambda (design) 6)";
  propertylist[4] = "(lambda (design) 3)";
  propertylist[5] = "(lambda (design) 1)";
  comp->setPropertyList(propertylist);
  ds->addComponent(comp);

  

  ObjectDataManager* obdm = game->getObjectDataManager();
  obdm->addNewObjectType(new Universe());
  obdm->addNewObjectType(new EmptyObject());
  obdm->addNewObjectType(new EmptyObject());
  obdm->addNewObjectType(new Planet());
  obdm->addNewObjectType(new Fleet());


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
  
  IGObject* universe = new IGObject();
  game->addObject(universe);
  universe->setType(obT_Universe);
  universe->setSize(100000000000ll);
  universe->setName("The Universe");
  universe->setPosition(Vector3d(0ll, 0ll, 0ll));
  universe->setVelocity(Vector3d(0ll, 0ll, 0ll));

  //add contained objects
  IGObject *mw_galaxy = new IGObject();
  game->addObject(mw_galaxy);
  mw_galaxy->setSize(10000000000ll);
  mw_galaxy->setType(obT_Galaxy);
  mw_galaxy->setName("Milky Way Galaxy");
  mw_galaxy->setPosition(Vector3d(0ll, -6000ll, 0ll));
  mw_galaxy->setVelocity(Vector3d(0ll, 0ll, 0ll));
  
  universe->addContainedObject(mw_galaxy->getID());
  // star system 1
  IGObject *sol = new IGObject();
  game->addObject(sol);
  sol->setSize(1400000ll);
  sol->setType(obT_Star_System);
  sol->setName("Sol/Terra System");
  sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
  sol->setVelocity(Vector3d(0ll, 0ll, 0ll));
  
  mw_galaxy->addContainedObject(sol->getID());
  // star system 2
  IGObject *ac = new IGObject();
  game->addObject(ac);
  ac->setSize(800000ll);
  ac->setType(obT_Star_System);
  ac->setName("Alpha Centauri System");
  ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
  ac->setVelocity(Vector3d(0ll, 0ll, 0ll));
  
  mw_galaxy->addContainedObject(ac->getID());
  // star system 3
  IGObject *sirius = new IGObject();
  game->addObject(sirius);
  sirius->setSize(2000000ll);
  sirius->setType(obT_Star_System);
  sirius->setName("Sirius System");
  sirius->setPosition(Vector3d(-250000000ll, -4000000000ll, 0ll));
  sirius->setVelocity(Vector3d(0ll, 0ll, 0ll));
  
  mw_galaxy->addContainedObject(sirius->getID());
  
  // now for some planets
  
  IGObject *earth = new IGObject();
  game->addObject(earth);
  earth->setSize(2);
  earth->setType(obT_Planet);
  earth->setName("Earth/Terra");
  earth->setPosition(sol->getPosition() + Vector3d(14960ll, 0ll, 0ll));
  sol->addContainedObject(earth->getID());
  
  IGObject *venus = new IGObject();
  game->addObject(venus);
  venus->setSize(2);
  venus->setType(obT_Planet);
  venus->setName("Venus");
  venus->setPosition(sol->getPosition() + Vector3d(0ll, 10800ll, 0ll));
  sol->addContainedObject(venus->getID());
  
  IGObject *mars = new IGObject();
  game->addObject(mars);
  mars->setSize(1);
  mars->setType(obT_Planet);
  mars->setName("Mars");
  mars->setPosition(sol->getPosition() + Vector3d(-22790ll, 0ll, 0ll));
  sol->addContainedObject(mars->getID());
  
  IGObject *acprime = new IGObject();
  game->addObject(acprime);
  acprime->setSize(2);
  acprime->setType(obT_Planet);
  acprime->setName("Alpha Centauri Prime");
  acprime->setPosition(ac->getPosition() + Vector3d(-6300ll, 78245ll, 0ll));
  ac->addContainedObject(acprime->getID());
  
  IGObject *s1 = new IGObject();
  game->addObject(s1);
  s1->setSize(2);
  s1->setType(obT_Planet);
  s1->setName("Sirius 1");
  s1->setPosition(sirius->getPosition() + Vector3d(45925ll, -34262ll, 0ll));
  sirius->addContainedObject(s1->getID());
  
  game->setCombatStrategy(new RSPCombat());
  
}

void MiniSec::startGame(){
  Game::getGame()->setTurnLength(600);
}

void MiniSec::doOnceATurn(){
  Game* game = Game::getGame();
  std::set<unsigned int> vis = game->getObjectIds();
  std::set<unsigned int> players = game->getPlayerIds();
  for(std::set<unsigned int>::iterator itplayer = players.begin(); 
      itplayer != players.end(); ++itplayer){
    game->getPlayer(*itplayer)->setVisibleObjects(vis);
  }
}

bool MiniSec::onAddPlayer(Player* player){
  Game *game = Game::getGame();
  char* name = player->getName();
  IGObject *star = new IGObject();
  game->addObject(star);
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
  
  game->getObject(1)->addContainedObject(star->getID());
  
  IGObject *planet = new IGObject();
  game->addObject(planet);
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
  
  star->addContainedObject(planet->getID());
  
  IGObject *fleet = new IGObject();
  game->addObject(fleet);
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
  ((Fleet*)(fleet->getObjectData()))->addShips(0, 2);
  fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
  
  star->addContainedObject(fleet->getID());
  
  
  return true;
}

void MiniSec::onPlayerAdded(Player* player){
  player->setVisibleObjects(Game::getGame()->getObjectIds());

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
  std::list<unsigned int> cl;
  cl.push_back(1);
  design->setComponents(cl);
  Game::getGame()->getDesignStore(1)->addDesign(design);

  design = new Design();
  design->setCategoryId(1);
  design->setName("Frigate");
  design->setDescription("Frigate ship");
  design->setOwner(player->getID());
  cl.clear();
  cl.push_back(2);
  design->setComponents(cl);
  Game::getGame()->getDesignStore(1)->addDesign(design);

  design = new Design();
  design->setCategoryId(1);
  design->setName("Battleship");
  design->setDescription("Battleship ship");
  design->setOwner(player->getID());
  cl.clear();
  cl.push_back(3);
  design->setComponents(cl);
  Game::getGame()->getDesignStore(1)->addDesign(design);

  //remove temporarily added usable components
  player->removeUsableComponent(1);
  player->removeUsableComponent(2);
  player->removeUsableComponent(3);
}
