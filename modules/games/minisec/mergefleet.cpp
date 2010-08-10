/*  MergeFleet Order
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/player.h>
#include <tpserver/message.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>

#include "mergefleet.h"

MergeFleet::MergeFleet() : Order(){
  name = "Merge Fleet";
  description = "Merge this fleet into another one";
}

MergeFleet::~MergeFleet(){
}

bool MergeFleet::doOrder(IGObject::Ptr ob){
  IGObject::Ptr parent = Game::getGame()->getObjectManager()->getObject(ob->getParent());
  
  Fleet *myfleet = (Fleet*)(ob->getObjectBehaviour());
  //find fleet to merge with
  
  uint32_t targetid = 0;
  
  std::set<uint32_t> oblist = parent->getContainedObjects();
  for(std::set<uint32_t>::iterator itcurr = oblist.begin(); itcurr != oblist.end(); ++itcurr){
    if(*itcurr == ob->getID())
      continue;
    IGObject::Ptr ptarget = Game::getGame()->getObjectManager()->getObject(*itcurr);
    if(ptarget->getType() == ob->getType()){
      Fleet* pfleet = (Fleet*)(ptarget->getObjectBehaviour());
      if(pfleet->getOwner() == myfleet->getOwner()){
        if(pfleet->getSize() + myfleet->getSize() > pfleet->getPosition().getDistance(myfleet->getPosition())){
          targetid = *itcurr;
          Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
          break;
        }
      }
    }
    Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
  }

  if(targetid == 0){
    Message::Ptr msg( new Message() );
    msg->setSubject("Merge Fleet order canceled");
    msg->setBody("No target fleet at this location");
    msg->addReference(rst_Action_Order, rsorav_Canceled);
    msg->addReference(rst_Object, ob->getID());
    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectBehaviour()))->getOwner())->postToBoard(msg);
  }else{
  
    IGObject::Ptr target = Game::getGame()->getObjectManager()->getObject(targetid);

    Message::Ptr msg( new Message() );
    msg->setSubject("Merge Fleet order complete");
    msg->setBody("The two fleets have been merged");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, target->getID());

    
    Fleet *tfleet = (Fleet*)(target->getObjectBehaviour());
      

    IdMap ships = myfleet->getShips();
    for(IdMap::iterator itcurr = ships.begin();
        itcurr != ships.end(); ++itcurr){
      tfleet->addShips(itcurr->first, itcurr->second);
    }

    //remove the fleet from the physical universe
    ob->removeFromParent();
    
    Game::getGame()->getObjectManager()->scheduleRemoveObject(ob->getID());
    Game::getGame()->getPlayerManager()->getPlayer(myfleet->getOwner())->getPlayerView()->removeOwnedObject(ob->getID());
	
    Game::getGame()->getObjectManager()->doneWithObject(target->getID());

    Game::getGame()->getPlayerManager()->getPlayer(myfleet->getOwner())->postToBoard(msg);
  }
  return true;
}

Order* MergeFleet::clone() const{
  MergeFleet * nmf = new MergeFleet();
  nmf->type = type;
  return nmf;
}
