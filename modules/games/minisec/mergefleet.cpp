/*  MergeFleet Order
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/result.h>
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
#include <tpserver/objectorderparameter.h>

#include "mergefleet.h"

MergeFleet::MergeFleet() : Order(){
  moveorder = new Move();
  
  name = "Merge Fleet";
  description = "Merge this fleet into another one";
  
  object = new ObjectOrderParameter();
  object->setName("fleet");
  object->setDescription("The fleet to be merged into");
  parameters.push_back(object);
  
}

MergeFleet::~MergeFleet(){
  delete moveorder;
  delete object;
}

void MergeFleet::createFrame(Frame * f, int objID, int pos){
  
  turns = moveorder->getETA(Game::getGame()->getObjectManager()->getObject(objID)); // number of turns
  Game::getGame()->getObjectManager()->doneWithObject(objID);
  
  Order::createFrame(f, objID, pos);
}

Result MergeFleet::inputFrame(Frame * f, unsigned int playerid){
  Result r = Order::inputFrame(f, playerid);
  if(!r) return r;

  IGObject* target = Game::getGame()->getObjectManager()->getObject(object->getObjectId());

  if(target == NULL || (object->getObjectId() != 0 && ((target->getType() != 4)
    || ((unsigned int)(((Fleet*)(target->getObjectData()))->getOwner())) != playerid))){
    Logger::getLogger()->debug("Player tried to merge fleet with something that is not a fleet");
    Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
    return Failure("Player tried to merge fleet with something that is not a fleet.");
  }
  moveorder->setDest(target->getPosition());
  Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
  return Success();
  
}


bool MergeFleet::doOrder(IGObject * ob){
    IGObject* target = Game::getGame()->getObjectManager()->getObject(object->getObjectId());
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
  Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());

  if(moveorder->doOrder(ob)){

    Fleet *myfleet = (Fleet*)(ob->getObjectData());

    Message * msg = new Message();
    msg->setSubject("Merge Fleet order complete");
    msg->setBody("The two fleets have been merged");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, object->getObjectId());

    if(object->getObjectId() != 0){
      Fleet *tfleet = (Fleet*)(Game::getGame()->getObjectManager()->getObject(object->getObjectId())->getObjectData());
      
      if(tfleet->getOwner() == myfleet->getOwner()){
	std::map<int, int> ships = myfleet->getShips();
	for(std::map<int, int>::iterator itcurr = ships.begin();
	    itcurr != ships.end(); ++itcurr){
	  tfleet->addShips(itcurr->first, itcurr->second);
	}

	Game::getGame()->getObjectManager()->scheduleRemoveObject(ob->getID());
	
      }
    Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
    }

    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);
    
    return true;
  }else{
    return false;
  }
}

Order* MergeFleet::clone() const{
  MergeFleet * nmf = new MergeFleet();
  nmf->type = type;
  return nmf;
}
