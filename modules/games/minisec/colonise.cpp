/*  Colonise order
 *
 *  Copyright (C) 2004-2005,2007  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/message.h>
#include "fleet.h"
#include "planet.h"
#include <tpserver/player.h>
#include "move.h"
#include <tpserver/combatstrategy.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>
#include <tpserver/objectparameter.h>

#include "colonise.h"

Colonise::Colonise() : Order(){
  moveorder = new Move();
  
  name = "Colonise";
  description = "Attempt to colonise a planet";
  
  object = new ObjectParameter();
  object->setName("planet");
  object->setDescription("The target planet to be colonised");
  parameters.push_back(object);
}

Colonise::~Colonise(){
  delete moveorder;
  delete object;
}

void Colonise::createFrame(Frame * f, int objID, int pos){
  turns = moveorder->getETA(Game::getGame()->getObjectManager()->getObject(objID)); // number of turns
  Game::getGame()->getObjectManager()->doneWithObject(objID);
  
  Order::createFrame(f, objID, pos);
  
}

bool Colonise::inputFrame(Frame * f, unsigned int playerid){
  
  if(!(Order::inputFrame(f, playerid)))
    return false;

  IGObject* target = Game::getGame()->getObjectManager()->getObject(object->getObjectId());

  if(target == NULL || (object->getObjectId() != 0 && target->getType() != 3)){
    Logger::getLogger()->debug("Player trying to colonise something that is not a planet");
    Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
    return false;
  }
  moveorder->setDest(target->getPosition());
  Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
  return true;
}

bool Colonise::doOrder(IGObject * ob){
  //if not close, move
  if(object->getObjectId() == 0)
    return true;

  if(moveorder->doOrder(ob)){

    Message * msg = new Message();
    msg->addReference(rst_Object, ob->getID());

    Fleet *fleet = (Fleet*)ob->getObjectData();
    Planet *planet = (Planet*)(Game::getGame()->getObjectManager()->getObject(object->getObjectId())->getObjectData());
    
    if(planet->getOwner() != fleet->getOwner()){
      
      if(planet->getOwner() != 0){
	//combat
	CombatStrategy * combat = Game::getGame()->getCombatStrategy();
	combat->setCombatants(ob, Game::getGame()->getObjectManager()->getObject(object->getObjectId()));
	combat->doCombat();
      }

      DesignStore* ds = Game::getGame()->getDesignStore();
      int shiptype = 0;
      int shiphp = 2000000;
      std::map<int, int> ships = fleet->getShips();
      unsigned int colonisePropID = ds->getPropertyByName( "Colonise");
      unsigned int armorPropID = ds->getPropertyByName( "Armour");
      for(std::map<int, int>::iterator itcurr = ships.begin();
	  itcurr != ships.end(); ++itcurr){
	Design *design = ds->getDesign(itcurr->first);
	if(design->getPropertyValue(colonisePropID) != 0.0 && shiphp > (int)design->getPropertyValue(armorPropID)){
	  shiptype = itcurr->first;
	  shiphp = (int)design->getPropertyValue(armorPropID);
	}
      }
      
      if(shiptype != 0){
	planet->setOwner(fleet->getOwner());
	
	fleet->removeShips(shiptype, 1);
	
	msg->setSubject("Colonised planet");
	msg->setBody("You have colonised a planet!");
	msg->addReference(rst_Action_Order, rsorav_Completion);
	msg->addReference(rst_Object, object->getObjectId());

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
    Game::getGame()->getObjectManager()->doneWithObject(object->getObjectId());
    return true;
  }else{
    return false;
  }
}

uint32_t Colonise::getPlanetId() const{
    return object->getObjectId();
}

Order* Colonise::clone() const{
  Colonise* nc = new Colonise();
  nc->type = type;
  return nc;
}
