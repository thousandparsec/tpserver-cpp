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
#include "game.h"
#include "logging.h"
#include "message.h"
#include "fleet.h"
#include "planet.h"
#include "player.h"
#include "move.h"
#include "combatstrategy.h"

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
  f->packInt(moveorder->getETA(Game::getGame()->getObject(objID))); // number of turns
  f->packInt(0); // size of resource list
  f->packInt(planetid);
  
}

bool Colonise::inputFrame(Frame * f){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  planetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObject(planetid);

  if(target == NULL || (planetid != 0 && target->getType() != 3)){
    Logger::getLogger()->debug("Player trying to colonise something that is not a planet");
    return false;
  }
  moveorder->setDest(target->getPosition());
  
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
    Planet *planet = (Planet*)(Game::getGame()->getObject(planetid)->getObjectData());
    
    if(planet->getOwner() != fleet->getOwner()){
      
      if(planet->getOwner() != -1){
	//combat
	CombatStrategy * combat = Game::getGame()->getCombatStrategy();
	combat->setCombatants(ob, Game::getGame()->getObject(planetid));
	combat->doCombat();
      }
	
	
      if(fleet->numShips(1) >= 1){
	planet->setOwner(fleet->getOwner());
	
	fleet->removeShips(1, 1);
	
	msg->setSubject("Colonised planet");
	msg->setBody("You have colonised a planet!");
	msg->addReference(rst_Action_Order, rsorav_Completion);
	msg->addReference(rst_Object, planetid);

      }else{
	msg->setSubject("Colonisation failed");
	msg->setBody("Your fleet did not have a frigate to colonise the planet");
	msg->addReference(rst_Action_Order, rsorav_Invalid);
      }
      
      if(fleet->numShips(0) == 0 && fleet->numShips(1) == 0 && fleet->numShips(2) == 0){
	Game::getGame()->scheduleRemoveObject(ob->getID());
      }
      
    }else{
      msg->setSubject("Colonisation failed");
      msg->setBody("You already own the planet you tried to colonise");
      msg->addReference(rst_Action_Order, rsorav_Canceled);
    }
    
    Game::getGame()->getPlayer(fleet->getOwner())->postToBoard(msg);

    return true;
  }else{
    return false;
  }
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
