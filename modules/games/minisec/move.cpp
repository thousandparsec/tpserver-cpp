/*  Move order
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

#include <tpserver/result.h>
#include <tpserver/order.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/player.h>
#include "fleet.h"
#include <tpserver/message.h>
#include <tpserver/playermanager.h>
#include <tpserver/spacecoordparam.h>
#include "minisecturn.h"
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>

#include "move.h"

Move::Move() : Order()
{
  name = "Move";
  description = "Move to a given position absolute in space";
  
  coords = new SpaceCoordParam();
  coords->setName("pos");
  coords->setDescription("The position in space to move to");
  addOrderParameter(coords);
}

Move::~Move(){
}

Vector3d Move::getDest() const
{
	return coords->getPosition();
}


void Move::setDest(const Vector3d & ndest)
{
  coords->setPosition(ndest);
}

int Move::getETA(IGObject *ob) const{
  Fleet* fleet = ((Fleet*)(ob->getObjectBehaviour()));
  uint64_t distance = coords->getPosition().getDistance(fleet->getPosition());
  uint32_t max_speed = fleet->maxSpeed();
  
  if(distance == 0) 
    return 1;
  return (int)((distance - 1) / max_speed) + 1;
}

void Move::createFrame(Frame * f, int pos)
{
  Game* game = Game::getGame();
  IGObject* obj = game->getObjectManager()->getObject(game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  if(obj != NULL){
    turns = getETA(obj);
    game->getObjectManager()->doneWithObject(obj->getID());
  }else{
    turns = 0;
    Logger::getLogger()->error("Move create frame: object not found, id = %d", obj->getID());
  }
  
  Order::createFrame(f, pos);	
}

Result Move::inputFrame(Frame * f, uint32_t playerid)
{
  return Order::inputFrame(f, playerid);
}

bool Move::doOrder(IGObject * ob){
  Vector3d dest = coords->getPosition();
  Fleet* fleet = ((Fleet*)(ob->getObjectBehaviour()));
  uint64_t distance = dest.getDistance(fleet->getPosition());
  uint64_t max_speed = fleet->maxSpeed();

  Logger::getLogger()->debug("Object(%d)->Move->doOrder(): Moving %lld at %lld speed (will take about %lld turns)", 
	ob->getID(), distance, max_speed, distance/max_speed);
  if(distance <= max_speed){
    uint32_t parentid;

    Logger::getLogger()->debug("Object(%d)->Move->doOrder(): Is arriving at [%lld, %lld, %lld] ", 
      ob->getID(), dest.getX(), dest.getY(), dest.getZ());
  
    fleet->setVelocity(Vector3d(0,0,0));
    parentid = ob->getParent();

    // recontainerise if necessary
    int containertype = Game::getGame()->getObjectManager()->getObject(parentid)->getContainerType();
    Game::getGame()->getObjectManager()->doneWithObject(parentid);
  
    if(fleet->getPosition() != dest && containertype >= 1){
      //removeFromParent();
      std::set<uint32_t> oblist = ((MinisecTurn*)(Game::getGame()->getTurnProcess()))->getContainerIds();
      for(std::set<uint32_t>::reverse_iterator itcurr = oblist.rbegin(); itcurr != oblist.rend(); ++itcurr){
        IGObject* testedobject = Game::getGame()->getObjectManager()->getObject(*itcurr);
        
        Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(testedobject->getParameterByType(obpT_Position_3D));
        SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(testedobject->getParameterByType(obpT_Size));
        if(pos == NULL || size == NULL){
          Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
          continue;
        }
        Vector3d pos1 = pos->getPosition();
        uint64_t size1 = size->getSize();
        
        uint64_t diff = dest.getDistance(pos1);
        if(diff <= fleet->getSize() / 2 + size1 / 2){
        
          Logger::getLogger()->debug("Container object %d", *itcurr);
          //if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
          //if(*itcurr != id){
          
          if(size1 >= fleet->getSize()){
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
    
    fleet->setPosition(dest);
    
    Message * msg = new Message();
    msg->setSubject("Move order complete");
    msg->setBody("The fleet '" +  ob->getName() + "' has reached it's destination.");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    msg->addReference(rst_Object, parentid); /* It's parent */
    Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->postToBoard(msg);

    return true;

  }else{
    Vector3d velo = (dest - fleet->getPosition()).makeLength(max_speed);
    Vector3d arriveat = fleet->getPosition()+velo;
    Logger::getLogger()->debug("Move->doOrder(%d): Velocity is [%lld, %lld, %lld] (will arrive at [%lld, %lld, %lld])", 
      ob->getID(), velo.getX(), velo.getY(), velo.getZ(), arriveat.getX(), arriveat.getY(), arriveat.getZ());

    fleet->setVelocity(velo);

    uint32_t parentid = ob->getParent();
    // recontainerise if necessary
    uint32_t containertype = Game::getGame()->getObjectManager()->getObject(parentid)->getContainerType();
    Game::getGame()->getObjectManager()->doneWithObject(parentid);
  
    if(fleet->getPosition() != arriveat && containertype >= 1){
      //removeFromParent();
      std::set<uint32_t> oblist = ((MinisecTurn*)(Game::getGame()->getTurnProcess()))->getContainerIds();
      for(std::set<uint32_t>::reverse_iterator itcurr = oblist.rbegin(); itcurr != oblist.rend(); ++itcurr){
        IGObject* testedobject = Game::getGame()->getObjectManager()->getObject(*itcurr);
        
        Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(testedobject->getParameterByType(obpT_Position_3D));
        SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(testedobject->getParameterByType(obpT_Size));
        if(pos == NULL || size == NULL){
          Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
          continue;
        }
        Vector3d pos1 = pos->getPosition();
        uint64_t size1 = size->getSize();
        
        uint64_t diff = arriveat.getDistance(pos1);
        if(diff <= fleet->getSize() / 2 + size1 / 2){
        
          Logger::getLogger()->debug("Container object %d", *itcurr);
          //if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
          //if(*itcurr != id){
          
          if(size1 >= fleet->getSize()){
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
    
    fleet->setPosition(arriveat);
    
    return false;
  }
}

Order* Move::clone() const{
  Move *nm = new Move();
  nm->type = type;
  return nm;
}
