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

#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"
#include "fleet.h"
#include "move.h"
#include "player.h"
#include "message.h"

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
  moveorder->setDest(Game::getGame()->getObject(fleetid)->getPosition());
  f->packInt(moveorder->getETA(Game::getGame()->getObject(objID))); // number of turns
  f->packInt(0); // size of resource list
  f->packInt(fleetid);
}

bool MergeFleet::inputFrame(Frame * f, unsigned int playerid){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  fleetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObject(fleetid);

  if(target == NULL || (fleetid != 0 && target->getType() != 4) || (fleetid != 0 && ((unsigned int)(((Fleet*)(target->getObjectData()))->getOwner())) != playerid){
    Logger::getLogger()->debug("Player tried to merge fleet with something that is not a fleet");
    return false;
  }
  moveorder->setDest(target->getPosition());

  return true;
}


bool MergeFleet::doOrder(IGObject * ob){
  moveorder->setDest(Game::getGame()->getObject(fleetid)->getPosition());
  
  if(moveorder->doOrder(ob)){

    Fleet *myfleet = (Fleet*)(ob->getObjectData());

    Message * msg = new Message();
    msg->setSubject("Merge Fleet order complete");
    msg->setBody("The two fleets have been merged");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, fleetid);

    if(fleetid != 0){
      Fleet *tfleet = (Fleet*)(Game::getGame()->getObject(fleetid)->getObjectData());
      
      if(tfleet->getOwner() == myfleet->getOwner()){
	std::map<int, int> ships = myfleet->getShips();
	for(std::map<int, int>::iterator itcurr = ships.begin();
	    itcurr != ships.end(); ++itcurr){
	  tfleet->addShips(itcurr->first, itcurr->second);
	}

	Game::getGame()->scheduleRemoveObject(ob->getID());
	
      }
    }

    Game::getGame()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);
    
    return true;
  }else{
    return false;
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
