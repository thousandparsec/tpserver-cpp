/*  MergeFleet Order
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

#include <tpserver/order.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include "fleet.h"
#include "move.h"
#include <tpserver/player.h>
#include <tpserver/message.h>
#include <tpserver/playermanager.h>

#include "mergefleet.h"

MergeFleet::MergeFleet() : Order(){
  type = odT_Fleet_Merge;
  moveorder = new Move();
}

MergeFleet::~MergeFleet(){
  delete moveorder;
}

void MergeFleet::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
    IGObject* target = Game::getGame()->getObjectManager()->getObject(fleetid);
    if(target != NULL){
        moveorder->setDest(target->getPosition());
        Game::getGame()->getObjectManager()->doneWithObject(fleetid);
         f->packInt(moveorder->getETA(Game::getGame()->getObjectManager()->getObject(objID))); // number of turns
        Game::getGame()->getObjectManager()->doneWithObject(objID);
    }else{
        f->packInt(0); // number of turns
    }
  f->packInt(0); // size of resource list
  f->packInt(fleetid);
}

bool MergeFleet::inputFrame(Frame * f, unsigned int playerid){
  f->unpackInt(); // number of turns
  int ressize = f->unpackInt(); // size of resource list (should be zero)
  for(int i = 0; i < ressize; i++){
    f->unpackInt(); //The resource id
    f->unpackInt(); //The amount of the resource
  }
  fleetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObjectManager()->getObject(fleetid);

  if(target == NULL || (fleetid != 0 && 
			((target->getType() != 4) || 
			((unsigned int)(((Fleet*)(target->getObjectData()))->getOwner())) != playerid))){
    Logger::getLogger()->debug("Player tried to merge fleet with something that is not a fleet");
    return false;
  }
  moveorder->setDest(target->getPosition());
    Game::getGame()->getObjectManager()->doneWithObject(fleetid);
  return true;
}


bool MergeFleet::doOrder(IGObject * ob){
    IGObject* target = Game::getGame()->getObjectManager()->getObject(fleetid);
    if(target == NULL){
        Message * msg = new Message();
        msg->setSubject("Merge Fleet order canceled");
        msg->setBody("The target fleet doesn't exist");
        msg->addReference(rst_Action_Order, rsorav_Canceled);
        msg->addReference(rst_Object, ob->getID());
        Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);
        return true;
    }
    moveorder->setDest(target->getPosition());
  Game::getGame()->getObjectManager()->doneWithObject(fleetid);

  if(moveorder->doOrder(ob)){

    Fleet *myfleet = (Fleet*)(ob->getObjectData());

    Message * msg = new Message();
    msg->setSubject("Merge Fleet order complete");
    msg->setBody("The two fleets have been merged");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, fleetid);

    if(fleetid != 0){
      Fleet *tfleet = (Fleet*)(Game::getGame()->getObjectManager()->getObject(fleetid)->getObjectData());
      
      if(tfleet->getOwner() == myfleet->getOwner()){
	std::map<int, int> ships = myfleet->getShips();
	for(std::map<int, int>::iterator itcurr = ships.begin();
	    itcurr != ships.end(); ++itcurr){
	  tfleet->addShips(itcurr->first, itcurr->second);
	}

	Game::getGame()->getObjectManager()->scheduleRemoveObject(ob->getID());
	
      }
    Game::getGame()->getObjectManager()->doneWithObject(fleetid);
    }

    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);
    
    return true;
  }else{
    return false;
  }
}

uint32_t MergeFleet::getFleetId() const{
    return fleetid;
}

void MergeFleet::setFleetId(uint32_t nfi){
    fleetid = nfi;

    IGObject* target = Game::getGame()->getObjectManager()->getObject(fleetid);
    if(target != NULL){
        moveorder->setDest(target->getPosition());
        Game::getGame()->getObjectManager()->doneWithObject(fleetid);
    }
}

void MergeFleet::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("MergeFleet");
  f->packString("Merge this fleet into another one");
  f->packInt(1);
  f->packString("fleet");
  f->packInt(opT_Object_ID);
  f->packString("The fleet to be merged into");
  f->packInt64(descmodtime);

}

Order* MergeFleet::clone() const{
  return new MergeFleet();
}
