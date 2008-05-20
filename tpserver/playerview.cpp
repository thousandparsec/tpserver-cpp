/*  PlayerView object, holding the player's view of the universe
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

// #include "string.h"
#include <time.h>

#include "logging.h"
#include "frame.h"
#include "game.h"
#include "objectview.h"
#include "designstore.h"
#include "design.h"
#include "designview.h"
#include "component.h"
#include "componentview.h"
#include "persistence.h"

#include "playerview.h"

PlayerView::PlayerView() : pid(0), visibleObjects(), ownedObjects(),
    cacheObjects(), modlistObject(), currObjSeq(0),
    visibleDesigns(), usableDesigns(), cacheDesigns(),
    modlistDesign(), currDesignSeq(0),
    visibleComponents(), usableComponents(), cacheComponents(),
    modlistComp(), currCompSeq(0){
}

PlayerView::~PlayerView(){
}

void PlayerView::setPlayerId(uint32_t newid){
  pid = newid;
}

void PlayerView::doOnceATurn(){
  currObjSeq++;
  currDesignSeq++;
  currCompSeq++;
}


void PlayerView::addVisibleObject(ObjectView* obj){
  visibleObjects.insert(obj->getObjectId());
  cacheObjects[obj->getObjectId()] = obj;
  Game::getGame()->getPersistence()->saveObjectView(pid, obj);
  currObjSeq++;
}

ObjectView* PlayerView::getObjectView(uint32_t objid){
  if(visibleObjects.find(objid) != visibleObjects.end()){
    ObjectView* obj = cacheObjects[objid];
    if(obj == NULL){
      obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, objid);
      if(obj != NULL){
        cacheObjects[objid] = obj;
      }
    }
    return obj;
  }else{
    return NULL;
  }
}

void PlayerView::updateObjectView(uint32_t objid){
  ObjectView* obj = cacheObjects[objid];
  Game::getGame()->getPersistence()->saveObjectView(pid, obj);
}

void PlayerView::removeVisibleObject(uint32_t objid){
  ObjectView* obj = getObjectView(objid);
  if(obj != NULL){
    cacheObjects[objid]->setGone(true);
    Game::getGame()->getPersistence()->saveObjectView(pid, cacheObjects[objid]);
  }
}

bool PlayerView::isVisibleObject(unsigned int objid){
  if(visibleObjects.find(objid) != visibleObjects.end()){
    ObjectView* obj = cacheObjects[objid];
    if(obj == NULL){
      obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, objid);
      if(obj != NULL){
        cacheObjects[objid] = obj;
      }
    }
    return !(obj->isGone());
  }
  return false;
}

std::set<uint32_t> PlayerView::getVisibleObjects() const{
  return visibleObjects;
}

void PlayerView::addOwnedObject(uint32_t objid){
  ownedObjects.insert(objid);
  ObjectView* obj = getObjectView(objid);
  if(obj != NULL){
    obj->setCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveObjectView(pid, cacheObjects[objid]);
  }else{
    ObjectView* ov = new ObjectView();
    ov->setObjectId(objid);
    ov->setCompletelyVisible(true);
    addVisibleObject(ov);
  }
}

void PlayerView::removeOwnedObject(uint32_t objid){
  ownedObjects.erase(objid);
}

uint32_t PlayerView::getNumberOwnedObjects() const{
  return ownedObjects.size();
}

std::set<uint32_t> PlayerView::getOwnedObjects() const{
  return ownedObjects;
}

void PlayerView::processGetObject(uint32_t objid, Frame* frame){
  if(visibleObjects.find(objid) == visibleObjects.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Object");
  }else{
    ObjectView* object = getObjectView(objid);
    object->packFrame(frame, pid);
  }
}

void PlayerView::processGetObjectIds(Frame* in, Frame* out){
  Logger::getLogger()->debug("doing Get Object Ids frame");
  
  if(in->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    out->createFailFrame(fec_FrameError, "Get Object ids isn't supported in this protocol");
    return;
  }
  
  if((in->getDataLength() != 12 && in->getVersion() <= fv0_3) || (in->getDataLength() != 20 && in->getVersion() >= fv0_4)){
    out->createFailFrame(fec_FrameError, "Invalid frame");
    return;
  }
  
  uint32_t seqnum = in->unpackInt();
  uint32_t snum = in->unpackInt();
  uint32_t numtoget = in->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(in->getVersion() >= fv0_4){
    fromtime = in->unpackInt64();
  }
  
  if(seqnum != currObjSeq && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    modlistObject.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    modlistObject.clear();
    for(std::map<uint32_t, ObjectView*>::iterator itcurr = cacheObjects.begin();
        itcurr != cacheObjects.end(); ++itcurr){
      ObjectView* obj = itcurr->second;
      if(obj == NULL){
        obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, itcurr->first);
        if(obj != NULL){
          cacheObjects[itcurr->first] = obj;
        }
      }
      if((fromtime == UINT64_NEG_ONE && !(obj->isGone())) || obj->getModTime() > fromtime){
        modlistObject[itcurr->first] = obj->getModTime();
      }
    }
  }
  
  if(snum > modlistObject.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, modlistObject.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlistObject.size() - snum){
    numtoget = modlistObject.size() - snum;
  }
    
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
    
  out->setType(ft03_ObjectIds_List);
  out->packInt(currObjSeq);
  out->packInt(modlistObject.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = modlistObject.begin();
  std::advance(itcurr, snum);
  for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    out->packInt(itcurr->first);
    out->packInt64(itcurr->second);
  }
  
  if(out->getVersion() >= fv0_4){
    out->packInt64(fromtime);
  }
  
}

void PlayerView::addVisibleDesign(DesignView* design){
  design->setModTime(time(NULL));
  uint32_t designid = design->getDesignId();
  visibleDesigns.insert(designid);
  if(cacheDesigns.find(designid) != cacheDesigns.end()){
    if(cacheDesigns[designid] != NULL){
      delete cacheDesigns[designid];
    }
  }
  cacheDesigns[designid] = design;
  Game::getGame()->getPersistence()->saveDesignView(pid, design);
  currDesignSeq++;
}

void PlayerView::addUsableDesign(uint32_t designid){
  usableDesigns.insert(designid);
  if(visibleDesigns.find(designid) == visibleDesigns.end()){
    DesignView* designview = new DesignView();
    designview->setDesignId(designid);
    designview->setIsCompletelyVisible(true);
    addVisibleDesign(designview);
  }else{
    DesignView* design = cacheDesigns[designid];
    if(design == NULL){
      design = Game::getGame()->getPersistence()->retrieveDesignView(pid, designid);
      if(design != NULL){
        cacheDesigns[designid] = design;
      }
    }
    design->setIsCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveDesignView(pid, design);
    currCompSeq++;
  }
}

void PlayerView::removeUsableDesign(uint32_t designid){
  std::set<uint32_t>::iterator dicurr = usableDesigns.find(designid);
  if(dicurr != usableDesigns.end())
    usableDesigns.erase(dicurr);
}

bool PlayerView::isUsableDesign(uint32_t designid) const{
  return (usableDesigns.find(designid) != usableDesigns.end());
}

std::set<uint32_t> PlayerView::getUsableDesigns() const{
  return usableDesigns;
}

std::set<uint32_t> PlayerView::getVisibleDesigns() const{
  return visibleDesigns;
}

void PlayerView::processGetDesign(uint32_t designid, Frame* frame){
  if(visibleComponents.find(designid) == visibleDesigns.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Design");
  }else{
    DesignView* design = cacheDesigns.find(designid)->second;
    if(design == NULL){
      design = Game::getGame()->getPersistence()->retrieveDesignView(pid, designid);
      if(design != NULL){
        cacheDesigns[designid] = design;
      }
    }
    design->packFrame(frame);
  }
}

void PlayerView::processGetDesignIds(Frame* in, Frame* out){
  Logger::getLogger()->debug("doing Get Design Ids frame");
  
  if(in->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    out->createFailFrame(fec_FrameError, "Get Design ids isn't supported in this protocol");
    return;
  }
  
  if((in->getDataLength() != 12 && in->getVersion() <= fv0_3) || (in->getDataLength() != 20 && in->getVersion() >= fv0_4)){
    out->createFailFrame(fec_FrameError, "Invalid frame");
    return;
  }
  
  uint32_t seqnum = in->unpackInt();
  uint32_t snum = in->unpackInt();
  uint32_t numtoget = in->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(in->getVersion() >= fv0_4){
    fromtime = in->unpackInt64();
  }
  
  if(seqnum != currDesignSeq && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    modlistDesign.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    modlistDesign.clear();
    for(std::map<uint32_t, DesignView*>::iterator itcurr = cacheDesigns.begin();
        itcurr != cacheDesigns.end(); ++itcurr){
      DesignView* designv = itcurr->second;
      if(designv == NULL){
        designv = Game::getGame()->getPersistence()->retrieveDesignView(pid, itcurr->first);
        if(designv != NULL){
          cacheDesigns[itcurr->first] = designv;
        }
      }
      if(fromtime == UINT64_NEG_ONE || designv->getModTime() > fromtime){
        modlistDesign[itcurr->first] = designv->getModTime();
      }
    }
  }
  
  if(snum > modlistDesign.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, visibleDesigns.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlistDesign.size() - snum){
    numtoget = modlistDesign.size() - snum;
  }
    
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
    
  out->setType(ft03_DesignIds_List);
  out->packInt(currDesignSeq);
  out->packInt(modlistDesign.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = modlistDesign.begin();
  std::advance(itcurr, snum);
  for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    out->packInt(itcurr->first);
    out->packInt64(itcurr->second);
  }
  
  if(out->getVersion() >= fv0_4){
    out->packInt64(fromtime);
  }
  
}

void PlayerView::addVisibleComponent(ComponentView* comp){
  comp->setModTime(time(NULL));
  uint32_t compid = comp->getComponentId();
  visibleComponents.insert(compid);
  if(cacheComponents.find(compid) != cacheComponents.end()){
    if(cacheComponents[compid] != NULL){
      delete cacheComponents[compid];
    }
  }
  cacheComponents[compid] = comp;
  Game::getGame()->getPersistence()->saveComponentView(pid, comp);
  currCompSeq++;
}

void PlayerView::addUsableComponent(uint32_t compid){
  usableComponents.insert(compid);
  if(visibleComponents.find(compid) == visibleComponents.end()){
    ComponentView* compview = new ComponentView();
    compview->setComponentId(compid);
    compview->setCompletelyVisible(true);
    addVisibleComponent(compview);
  }else{
    ComponentView* compv = cacheComponents[compid];
    if(compv == NULL){
      compv = Game::getGame()->getPersistence()->retrieveComponentView(pid, compid);
      if(compv != NULL){
        cacheComponents[compid] = compv;
      }
    }
    compv->setCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveComponentView(pid, compv);
    currCompSeq++;
  }
}

void PlayerView::removeUsableComponent(uint32_t compid){
  std::set<uint32_t>::iterator cicurr = usableComponents.find(compid);
  if(cicurr != usableComponents.end())
    usableComponents.erase(cicurr);
}

bool PlayerView::isUsableComponent(uint32_t compid) const{
  return (usableComponents.find(compid) != usableComponents.end());
}

std::set<uint32_t> PlayerView::getVisibleComponents() const{
  return visibleComponents;
}

std::set<uint32_t> PlayerView::getUsableComponents() const{
  return usableComponents;
}

void PlayerView::processGetComponent(uint32_t compid, Frame* frame){
  if(visibleComponents.find(compid) == visibleComponents.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Component");
  }else{
    ComponentView* component = cacheComponents.find(compid)->second;
    if(component == NULL){
      component = Game::getGame()->getPersistence()->retrieveComponentView(pid, compid);
      if(component != NULL){
        cacheComponents[compid] = component;
      }
    }
    component->packFrame(frame);
  }
}

void PlayerView::processGetComponentIds(Frame* in, Frame* out){
  Logger::getLogger()->debug("doing Get Component Ids frame");
  
  if(in->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    out->createFailFrame(fec_FrameError, "Get Component ids isn't supported in this protocol");
    return;
  }
  
  if((in->getDataLength() != 12 && in->getVersion() <= fv0_3) || (in->getDataLength() != 20 && in->getVersion() >= fv0_4)){
    out->createFailFrame(fec_FrameError, "Invalid frame");
    return;
  }
  
  uint32_t seqnum = in->unpackInt();
  uint32_t snum = in->unpackInt();
  uint32_t numtoget = in->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(in->getVersion() >= fv0_4){
    fromtime = in->unpackInt64();
  }
  
  if(seqnum != currCompSeq && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    modlistComp.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    //clear current mod list in case it has stuff in it still
    modlistComp.clear();
    for(std::map<uint32_t, ComponentView*>::iterator itcurr = cacheComponents.begin();
        itcurr != cacheComponents.end(); ++itcurr){
      ComponentView* component = itcurr->second;
      if(component == NULL){
        component = Game::getGame()->getPersistence()->retrieveComponentView(pid, itcurr->first);
        if(component != NULL){
          cacheComponents[itcurr->first] = component;
        }
      }
      if(fromtime == UINT64_NEG_ONE || component->getModTime() > fromtime){
        modlistComp[itcurr->first] = component->getModTime();
      }
    }
  }
  
  if(snum > modlistComp.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, modlistComp.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlistComp.size() - snum){
    numtoget = modlistComp.size() - snum;
  }
  
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
  
  out->setType(ft03_ComponentIds_List);
  out->packInt(currCompSeq);
  out->packInt(modlistComp.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = modlistComp.begin();
  std::advance(itcurr, snum);
  for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    out->packInt(itcurr->first);
    out->packInt64(itcurr->second);
  }
  
  if(out->getVersion() >= fv0_4){
    out->packInt64(fromtime);
  }
  
}

void PlayerView::setVisibleObjects(const std::set<uint32_t>& obids){
  visibleObjects = obids;
}

void PlayerView::setOwnedObjects(const std::set<uint32_t>& obids){
  ownedObjects = obids;
}

void PlayerView::setVisibleDesigns(const std::set<uint32_t>& dids){
  visibleDesigns = dids;
}

void PlayerView::setUsableDesigns(const std::set<uint32_t>& dids){
  usableDesigns = dids;
}

void PlayerView::setVisibleComponents(const std::set<uint32_t>& cids){
  visibleComponents = cids;
}

void PlayerView::setUsableComponents(const std::set<uint32_t>& cids){
  usableComponents = cids;
}
