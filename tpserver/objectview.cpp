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

ObjectView::ObjectView(): objid(0), completelyvisible(false), gone(false), seename(false),
                              visiblename(), seedesc(false), visibledesc(){
  timestamp = time(NULL);
}

ObjectView::~ObjectView(){

}

void ObjectView::packFrame(Frame* frame, uint32_t playerid) const{
  if(gone){
    frame->createFailFrame(fec_NonExistant, "No such object");
  }else{
    frame->setType(ft02_Object);
    frame->packInt(objid);
    
    IGObject* object = Game::getGame()->getObjectManager()->getObject(objid);
    PlayerView* playerview = Game::getGame()->getPlayerManager()->getPlayer(playerid)->getPlayerView();
    
    //type
    frame->packInt(object->getType());
    if(completelyvisible || seename){
      frame->packString(object->getName());
    }else{
      frame->packString(visiblename);
    }
    if(frame->getVersion() >= fv0_4){
      if(completelyvisible || seedesc){
        frame->packString(object->getDescription());
      }else{
        frame->packString(visibledesc);
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
    
    std::set<uint32_t> children = object->getContainedObjects();
    std::set<uint32_t>::iterator itcurr, itend;
    itcurr = children.begin();
    itend = children.end();
    
    while(itcurr != itend){
      if(!playerview->isVisibleObject(*itcurr)){
        std::set<unsigned int>::iterator itemp = itcurr;
        ++itcurr;
        children.erase(itemp);
      }else{
        ++itcurr;
      }
    }
    
    frame->packInt(children.size());
    //for loop for children objects
    itend = children.end();
    for (itcurr = children.begin(); itcurr != itend; itcurr++) {
      frame->packInt(*itcurr);
    }
  
    if(frame->getVersion() <= fv0_3){
    
      
      OrderQueueObjectParam* oq = dynamic_cast<OrderQueueObjectParam*>(object->getParameterByType(obpT_Order_Queue));
      if(oq != NULL){
        OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(oq->getQueueId());
        if(queue->isOwner(playerid)){
          std::set<uint32_t> allowedtypes = queue->getAllowedOrderTypes();
          frame->packInt(allowedtypes.size());
          for(std::set<uint32_t>::iterator itcurr = allowedtypes.begin(); itcurr != allowedtypes.end();
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
  return objid;
}

bool ObjectView::isCompletelyVisible() const{
  return completelyvisible;
}

bool ObjectView::isGone() const{
  return gone;
}

bool ObjectView::canSeeName() const{
  return seename;
}

std::string ObjectView::getVisibleName() const{
  return visiblename;
}

bool ObjectView::canSeeDescription() const{
  return seedesc;
}

std::string ObjectView::getVisibleDescription() const{
    return visibledesc;
}

uint64_t ObjectView::getModTime() const{
    return timestamp;
}

void ObjectView::setObjectId(uint32_t id){
  objid = id;
  touchModTime();
}

void ObjectView::setCompletelyVisible(bool ncv){
  if(ncv != completelyvisible)
    touchModTime();
  completelyvisible = ncv;
}

void ObjectView::setGone(bool nid){
  if(nid != gone){
    touchModTime();
    gone = nid;
  }
}

void ObjectView::setCanSeeName(bool csn){
  if(csn != seename)
    touchModTime();
  seename = csn;
}

void ObjectView::setVisibleName(const std::string& nvn){
  visiblename = nvn;
  touchModTime();
}

void ObjectView::setCanSeeDescription(bool csd){
  if(csd != seedesc)
    touchModTime();
  seedesc = csd;
}

void ObjectView::setVisibleDescription(const std::string& nvd){
  visibledesc = nvd;
  touchModTime();
}


void ObjectView::setModTime(uint64_t nmt){
    timestamp = nmt;
}

void ObjectView::touchModTime(){
  timestamp = time(NULL);
}
