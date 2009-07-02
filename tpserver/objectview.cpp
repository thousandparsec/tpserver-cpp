/*  ObjectView class
 *
 *  Copyright (C) 2008 Lee Begg and the Thousand Parsec Project
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

#include <time.h>

#include "frame.h"
#include "game.h"
#include "objectmanager.h"
#include "object.h"
#include "objectbehaviour.h"
#include "playermanager.h"
#include "player.h"
#include "playerview.h"
#include "objectparametergroup.h"
#include "component.h"
#include "designstore.h"
#include "ordermanager.h"
#include "position3dobjectparam.h"
#include "velocity3dobjectparam.h"
#include "orderqueueobjectparam.h"
#include "orderqueue.h"
#include "sizeobjectparam.h"


#include "objectview.h"

ObjectView::ObjectView(): ProtocolView(ft02_Object), gone(false) { 
}
ObjectView::ObjectView( uint32_t new_id, bool visibility ) : ProtocolView(ft02_Object), gone(false) { 
  setId( new_id );
  setCompletelyVisible( visibility );
}

ObjectView::~ObjectView(){

}

void ObjectView::packFrame(Frame* frame, uint32_t playerid) const{
  IGObject* object = Game::getGame()->getObjectManager()->getObject(id);
  
  if(gone || (completely_visible && (object == NULL || !object->isAlive()))){
    frame->createFailFrame(fec_NonExistant, "No such object");
  }else{
    frame->setType(ft02_Object);
    frame->packInt(id);
    
    PlayerView* playerview = Game::getGame()->getPlayerManager()->getPlayer(playerid)->getPlayerView();
    
    //type
    frame->packInt(object->getType());
    if(completely_visible || name_visible){
      frame->packString(object->getName());
    }else{
    }
    if(frame->getVersion() >= fv0_4){
      if(completely_visible || desc_visible){
        frame->packString(object->getDescription());
      }else{
        frame->packString(desc);
      }
    }
    
    if(frame->getVersion() >= fv0_4){
      // should be moved to ObjectRelationshipsData::packFrame once TP03 support is removed
      frame->packInt(object->getParent());
    }else{
      //pre tp04
    
      SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(object->getParameterByType(obpT_Size));
      if(size != NULL){
        frame->packInt64(size->getSize());
      }else{
        frame->packInt64(0);
      }
    
      Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(object->getParameterByType(obpT_Position_3D));
      if(pos != NULL){
        pos->getPosition().pack(frame);
      }else{
        Vector3d(0,0,0).pack(frame);
      }
    
      Velocity3dObjectParam * vel = dynamic_cast<Velocity3dObjectParam*>(object->getParameterByType(obpT_Velocity));
      if(vel != NULL){
        vel->getVelocity().pack(frame);
      }else{
        Vector3d(0,0,0).pack(frame);
      }
    
    }
    
    IdSet children = object->getContainedObjects();
    IdSet::iterator itcurr, itend;
    itcurr = children.begin();
    itend = children.end();
    
    while(itcurr != itend){
      if(!playerview->isVisibleObject(*itcurr)){
        IdSet::iterator itemp = itcurr;
        ++itcurr;
        children.erase(itemp);
      }else{
        ++itcurr;
      }
    }
    
    frame->packIdSet(children);
  
    if(frame->getVersion() <= fv0_3){
    
      
      OrderQueueObjectParam* oq = dynamic_cast<OrderQueueObjectParam*>(object->getParameterByType(obpT_Order_Queue));
      if(oq != NULL){
        OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(oq->getQueueId());
        if(queue->isOwner(playerid)){
          IdSet allowedtypes = queue->getAllowedOrderTypes();
          frame->packInt(allowedtypes.size());
          for(IdSet::iterator itcurr = allowedtypes.begin(); itcurr != allowedtypes.end();
              ++itcurr){
            frame->packInt(*itcurr);
          }
          frame->packInt(queue->getNumberOrders());
        }else{
          frame->packInt(0);
          frame->packInt(0);
        }
      }else{
        frame->packInt(0);
        frame->packInt(0);
      }
        
    }
    frame->packInt64(getModTime());
    frame->packInt(0);
    frame->packInt(0);
    if(frame->getVersion() >= fv0_4){
      frame->packInt(0);
      frame->packInt(0);

      std::map<uint32_t, ObjectParameterGroupPtr> parameters = object->getParameterGroups();
      for(std::map<uint32_t, ObjectParameterGroupPtr>::iterator itcurr = parameters.begin(); itcurr != parameters.end();
          ++itcurr){
        (itcurr->second)->packObjectFrame(frame, playerid);
      }
    }else{
      ObjectBehaviour* behaviour = object->getObjectBehaviour();
      if(behaviour != NULL){
        behaviour->packExtraData(frame);
      }
    }

  }
}

uint32_t ObjectView::getObjectId() const{
  return id;
}

bool ObjectView::isGone() const{
  return gone;
}

void ObjectView::setObjectId(uint32_t id){
  setId( id );
  touchModTime();
}

void ObjectView::setGone(bool nid){
  if(nid != gone){
    touchModTime();
    gone = nid;
  }
}

