/*  Interception Order
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
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/player.h>
#include <tpserver/message.h>
#include <tpserver/playermanager.h>
#include <tpserver/orderparameters.h>
#include "minisecturn.h"
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderparameters.h>
#include <tpserver/objecttypemanager.h>

#include "planet.h"
#include "emptyobject.h"
#include "fleet.h"

#include "intercept.h"

Intercept::Intercept() : Order()
{
  name = "Intercept";
  description = "Intercept a given ship";
  
  uint32_t fleet_type = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Fleet");
  std::set<objecttypeid_t> fleet_set;
  fleet_set.insert(fleet_type);
  destination = (ObjectOrderParameter*) addOrderParameter(new ObjectOrderParameter( "Fleet", "The fleet to intercept", fleet_set) );
}

Intercept::~Intercept(){
}

void Intercept::createFrame(OutputFrame::Ptr f, int pos)
{
  Game* game = Game::getGame();
  IGObject::Ptr obj = game->getObjectManager()->getObject(game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  if(obj != NULL && destination->getObjectId() != 0){
    game->getObjectManager()->doneWithObject(obj->getID());
  }else{
    turns = 0;
    Logger::getLogger()->error("Intercept create frame: object not found, id = %d", obj->getID());
  }
  
  Order::createFrame(f, pos);	
}

void Intercept::inputFrame(InputFrame::Ptr f, uint32_t playerid)
{
  Order::inputFrame(f, playerid);
}

ObjectOrderParameter* Intercept::getDestination()
{
  return destination;
}

bool Intercept::doOrder(IGObject::Ptr ob){

  if (destination->getObjectId() == 0)
    return true;

  Fleet* me = dynamic_cast<Fleet*>(ob->getObjectBehaviour());
  if (me == NULL)
    return false;

  ObjectManager* om = Game::getGame()->getObjectManager();
  ObjectTypeManager* otm = Game::getGame()->getObjectTypeManager();
  IGObject::Ptr d = om->getObject(destination->getObjectId());

  SpaceObject* dest_object;

  if (d->getType() == otm->getObjectTypeByName("Fleet"))
    dest_object = dynamic_cast<Fleet*>(d->getObjectBehaviour());
  else if (d->getType() == otm->getObjectTypeByName("Planet"))
    dest_object = dynamic_cast<Planet*>(d->getObjectBehaviour());
  else if (d->getType() == otm->getObjectTypeByName("System"))
    dest_object = dynamic_cast<EmptyObject*>(d->getObjectBehaviour());
  else
    return false;

  Vector3d dest = dest_object->getPosition();
  Vector3d velocity = dest_object->getVelocity();
  Vector3d previous = dest - velocity;

  uint64_t my_speed = me->maxSpeed(); 
  uint64_t distance = dest.getDistance(me->getPosition());

  Logger::getLogger()->debug("Object(%d)->Intercept->doOrder(): Moving %lld at %lld speed (will take about %lld turns)", 
	ob->getID(), distance, my_speed, distance/my_speed);
  if(distance <= my_speed) {
    uint32_t parentid;

    Logger::getLogger()->debug("Object(%d)->Intercept->doOrder(): Is arriving at [%lld, %lld, %lld] ", 
      ob->getID(), dest.getX(), dest.getY(), dest.getZ());
  
    me->setVelocity(Vector3d(0,0,0));
    parentid = ob->getParent();

    // recontainerise if necessary
    int containertype = Game::getGame()->getObjectManager()->getObject(parentid)->getContainerType();
    Game::getGame()->getObjectManager()->doneWithObject(parentid);
  
    if(me->getPosition() != dest && containertype >= 1){
      //removeFromParent();
      std::set<uint32_t> oblist = ((MinisecTurn*)(Game::getGame()->getTurnProcess()))->getContainerIds();
      for(std::set<uint32_t>::reverse_iterator itcurr = oblist.rbegin(); itcurr != oblist.rend(); ++itcurr){
        IGObject::Ptr testedobject = Game::getGame()->getObjectManager()->getObject(*itcurr);
        
        Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(testedobject->getParameterByType(obpT_Position_3D));
        SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(testedobject->getParameterByType(obpT_Size));
        if(pos == NULL || size == NULL){
          Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
          continue;
        }
        Vector3d pos1 = pos->getPosition();
        uint64_t size1 = size->getSize();
        
        uint64_t diff = dest.getDistance(pos1);
        if(diff <= me->getSize() / 2 + size1 / 2){
        
          Logger::getLogger()->debug("Container object %d", *itcurr);
          //if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
          //if(*itcurr != id){
          
          if(size1 >= me->getSize()){
            if(*itcurr != parentid){
                ob->removeFromParent();
                ob->addToParent(*itcurr);
            }
            Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
            parentid = *itcurr;
            break;
          }
        }
        Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
        //}
      }
      if(parentid == 0){
          ob->removeFromParent();
          ob->addToParent(0);
      }
    }
    
    me->setPosition(dest);
    
    Message::Ptr msg( new Message() );
    msg->setSubject("Intercept order complete");
    msg->setBody("The fleet '" +  ob->getName() + "' has reached it's destination.");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, parentid); /* It's parent */
    Game::getGame()->getPlayerManager()->getPlayer(me->getOwner())->postToBoard(msg);

    return true;

  }else{

    int num_turns = 1;
    Vector3d calculated_pos = me->getPosition();

    while (dest.getDistance(calculated_pos) > my_speed*num_turns)
    {
      dest = dest + velocity;
      num_turns++;
      if (num_turns >= 50)
        return true;
    }
    turns = num_turns;

    Vector3d velo = (dest - me->getPosition()).makeLength(my_speed);
    Vector3d arriveat = me->getPosition()+velo;
    Logger::getLogger()->debug("Intercept->doOrder(%d): Velocity is [%lld, %lld, %lld] (will arrive at [%lld, %lld, %lld] in [%d] turns!)", 
      ob->getID(), velo.getX(), velo.getY(), velo.getZ(), arriveat.getX(), arriveat.getY(), arriveat.getZ(), num_turns);

    me->setVelocity(velo);

    uint32_t parentid = ob->getParent();
    // recontainerise if necessary
    uint32_t containertype = Game::getGame()->getObjectManager()->getObject(parentid)->getContainerType();
    Game::getGame()->getObjectManager()->doneWithObject(parentid);
  
    if(me->getPosition() != arriveat && containertype >= 1){
      //removeFromParent();
      std::set<uint32_t> oblist = ((MinisecTurn*)(Game::getGame()->getTurnProcess()))->getContainerIds();
      for(std::set<uint32_t>::reverse_iterator itcurr = oblist.rbegin(); itcurr != oblist.rend(); ++itcurr){
        IGObject::Ptr testedobject = Game::getGame()->getObjectManager()->getObject(*itcurr);
        
        Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(testedobject->getParameterByType(obpT_Position_3D));
        SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(testedobject->getParameterByType(obpT_Size));
        if(pos == NULL || size == NULL){
          Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
          continue;
        }
        Vector3d pos1 = pos->getPosition();
        uint64_t size1 = size->getSize();
        
        uint64_t diff = arriveat.getDistance(pos1);
        if(diff <= me->getSize() / 2 + size1 / 2){
        
          Logger::getLogger()->debug("Container object %d", *itcurr);
          //if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
          //if(*itcurr != id){
          
          if(size1 >= me->getSize()){
            if(*itcurr != parentid){
                ob->removeFromParent();
                ob->addToParent(*itcurr);
            }
            Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
            parentid = *itcurr;
            break;
          }
        }
        Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
        //}
      }
      if(parentid == 0){
          ob->removeFromParent();
          ob->addToParent(0);
      }
    }
    
    me->setPosition(arriveat);
    
    return false;
  }
}

Order* Intercept::clone() const{
  Intercept *nm = new Intercept();
  nm->type = type;
  return nm;
}
