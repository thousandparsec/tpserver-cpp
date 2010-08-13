/*  Colonise order
 *
 *  Copyright (C) 2004-2005,2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/objecttypemanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/message.h>
#include "fleet.h"
#include "planet.h"
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/ordermanager.h>

#include "colonise.h"

Colonise::Colonise() : Order(){
  
  name = "Colonise";
  description = "Attempt to colonise a planet at the current location";
  turns = 1;
}

Colonise::~Colonise(){
}

bool Colonise::doOrder(IGObject::Ptr ob){
  //if not close, move
  IGObject::Ptr target = Game::getGame()->getObjectManager()->getObject(ob->getParent());
  if(target == NULL || target->getType() != Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Planet")){
    
    Logger::getLogger()->debug("Object(%d)->Colonise->doOrder(): Target was not valid.", ob->getID());
    Game::getGame()->getObjectManager()->doneWithObject(ob->getParent());
    Message::Ptr msg( new Message() );
    msg->setSubject("Colonise order canceled");
    msg->setBody("Not at a planet, colonisation canceled");
    msg->addReference(rst_Action_Order, rsorav_Canceled);
    msg->addReference(rst_Object, ob->getID());
    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectBehaviour()))->getOwner())->postToBoard(msg);

    return true;
  }
  
  Fleet* fleet = (Fleet*)(ob->getObjectBehaviour());
  Planet* planet = (Planet*)(target->getObjectBehaviour());
  
    Message::Ptr msg( new Message() );
  msg->addReference(rst_Object, ob->getID());
  msg->addReference(rst_Object, target->getID());

  if(planet->getOwner() != fleet->getOwner()){

    if(planet->getOwner() != 0){
      Logger::getLogger()->debug("Object(%d)->Colonise->doOrder(): Planet already owned.", 
        ob->getID());
    
      msg->setSubject("Colonisation failed");
      msg->setBody("The planet you tried to colonise, is already owned by someone else.");
      msg->addReference(rst_Action_Order, rsorav_Canceled);
    }else{

      DesignStore::Ptr ds = Game::getGame()->getDesignStore();
      int shiptype = 0;
      int shiphp = 2000000;
      IdMap ships = fleet->getShips();
      uint32_t colonisePropID = ds->getPropertyByName( "Colonise");
      uint32_t armorPropID = ds->getPropertyByName( "Armour");
      for(IdMap::iterator itcurr = ships.begin();
	  itcurr != ships.end(); ++itcurr){
        Design::Ptr design = ds->getDesign(itcurr->first);
	if(design->getPropertyValue(colonisePropID) != 0.0 && shiphp > (int)design->getPropertyValue(armorPropID)){
	  shiptype = itcurr->first;
	  shiphp = (int)design->getPropertyValue(armorPropID);
	}
      }

      Logger::getLogger()->debug("Object(%d)->Colonise->doOrder(): shiptype %d", 
          ob->getID(), shiptype);
      if(shiptype != 0){
        uint32_t oldowner = planet->getOwner();
        planet->setOwner(fleet->getOwner());
        Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->getPlayerView()->addOwnedObject(target->getID());
        uint32_t queueid = static_cast<OrderQueueObjectParam*>(target->getParameterByType(obpT_Order_Queue))->getQueueId();
        OrderQueue::Ptr queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
        queue->removeOwner(oldowner);
        queue->addOwner(fleet->getOwner());

        fleet->removeShips(shiptype, 1);

        msg->setSubject("Colonised planet");
        msg->setBody("You have colonised a planet!");
        msg->addReference(rst_Action_Order, rsorav_Completion);

      }else{
        msg->setSubject("Colonisation failed");
        msg->setBody("Your fleet did not have a frigate to colonise the planet");
        msg->addReference(rst_Action_Order, rsorav_Invalid);
      }

      if(fleet->totalShips() == 0){
        ob->removeFromParent();
        Game::getGame()->getObjectManager()->scheduleRemoveObject(ob->getID());
        Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->getPlayerView()->removeOwnedObject(ob->getID());
      }
      
    }
  }else{
    Logger::getLogger()->debug("Object(%d)->Colonise->doOrder(): Was already owned by the player!", 
      ob->getID());

    msg->setSubject("Colonisation failed");
    msg->setBody("You already own the planet you tried to colonise");
    msg->addReference(rst_Action_Order, rsorav_Canceled);
  }
  
  Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->postToBoard(msg);
  Game::getGame()->getObjectManager()->doneWithObject(target->getID());
  return true;
  
}

Order* Colonise::clone() const{
  Colonise* nc = new Colonise();
  nc->type = type;
  return nc;
}
