/*  Colonise order
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

#include "order.h"
#include "frame.h"
#include "object.h"
#include "objectmanager.h"
#include "game.h"
#include "logging.h"
#include "message.h"
#include "fleet.h"
#include "planet.h"
#include "player.h"
#include "move.h"
#include "combatstrategy.h"
#include "design.h"
#include "designstore.h"
#include "playermanager.h"

#include "colonise.h"

Colonise::Colonise() : Order(){
  type = odT_Colonise;
  moveorder = new Move();
}

Colonise::~Colonise(){
  delete moveorder;
}

void Colonise::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
  f->packInt(moveorder->getETA(Game::getGame()->getObjectManager()->getObject(objID))); // number of turns
    Game::getGame()->getObjectManager()->doneWithObject(objID);
  f->packInt(0); // size of resource list
  f->packInt(planetid);
  
}

bool Colonise::inputFrame(Frame * f, unsigned int playerid){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  planetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObjectManager()->getObject(planetid);

  if(target == NULL || (planetid != 0 && target->getType() != 3)){
    Logger::getLogger()->debug("Player trying to colonise something that is not a planet");
    Game::getGame()->getObjectManager()->doneWithObject(planetid);
    return false;
  }
  moveorder->setDest(target->getPosition());
  Game::getGame()->getObjectManager()->doneWithObject(planetid);
  return true;
}

bool Colonise::doOrder(IGObject * ob){
  //if not close, move
  if(planetid == 0)
    return true;

  if(moveorder->doOrder(ob)){

    Message * msg = new Message();
    msg->addReference(rst_Object, ob->getID());

    Fleet *fleet = (Fleet*)ob->getObjectData();
    Planet *planet = (Planet*)(Game::getGame()->getObjectManager()->getObject(planetid)->getObjectData());
    
    if(planet->getOwner() != fleet->getOwner()){
      
      if(planet->getOwner() != -1){
	//combat
	CombatStrategy * combat = Game::getGame()->getCombatStrategy();
	combat->setCombatants(ob, Game::getGame()->getObjectManager()->getObject(planetid));
	combat->doCombat();
      }

      DesignStore* ds = Game::getGame()->getDesignStore();
      int shiptype = 0;
      int shiphp = 2000000;
      std::map<int, int> ships = fleet->getShips();
      for(std::map<int, int>::iterator itcurr = ships.begin();
	  itcurr != ships.end(); ++itcurr){
	Design *design = ds->getDesign(itcurr->first);
	if(design->getPropertyValue(6) != 0.0 && shiphp > (int)design->getPropertyValue(3)){
	  shiptype = itcurr->first;
	  shiphp = (int)design->getPropertyValue(3);
	}
      }
      
      if(shiptype != 0){
	planet->setOwner(fleet->getOwner());
	
	fleet->removeShips(shiptype, 1);
	
	msg->setSubject("Colonised planet");
	msg->setBody("You have colonised a planet!");
	msg->addReference(rst_Action_Order, rsorav_Completion);
	msg->addReference(rst_Object, planetid);

      }else{
	msg->setSubject("Colonisation failed");
	msg->setBody("Your fleet did not have a frigate to colonise the planet");
	msg->addReference(rst_Action_Order, rsorav_Invalid);
      }
      
      if(fleet->totalShips() == 0){
	Game::getGame()->getObjectManager()->scheduleRemoveObject(ob->getID());
      }
      
    }else{
      msg->setSubject("Colonisation failed");
      msg->setBody("You already own the planet you tried to colonise");
      msg->addReference(rst_Action_Order, rsorav_Canceled);
    }
    
    Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->postToBoard(msg);
    Game::getGame()->getObjectManager()->doneWithObject(planetid);
    return true;
  }else{
    return false;
  }
}

uint32_t Colonise::getPlanetId() const{
    return planetid;
}

void Colonise::setPlanetId(uint32_t npi){
    planetid = npi;

    IGObject* target = Game::getGame()->getObjectManager()->getObject(planetid);
    moveorder->setDest(target->getPosition());
    Game::getGame()->getObjectManager()->doneWithObject(planetid);
}

void Colonise::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("Colonise");
  f->packString("Attempt to colonise a planet");
  f->packInt(1);
  f->packString("planet");
  f->packInt(opT_Object_ID);
  f->packString("The target planet to be colonised");
  f->packInt64(descmodtime);
  
}

Order* Colonise::clone() const{
  return new Colonise();
}
