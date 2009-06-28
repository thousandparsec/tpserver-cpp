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

PlayerView::PlayerView() : pid(0){
}

PlayerView::~PlayerView(){
}

void PlayerView::setPlayerId(uint32_t newid){
  pid = newid;
}

void PlayerView::doOnceATurn(){
  objects.sequence++;
  designs.sequence++;
  components.sequence++;
}


void PlayerView::addVisibleObject(ObjectView* obj){
  objects.visible.insert(obj->getObjectId());
  objects.cache[obj->getObjectId()] = obj;
  Game::getGame()->getPersistence()->saveObjectView(pid, obj);
  objects.sequence++;
}

ObjectView* PlayerView::getObjectView(uint32_t objid){
  if(objects.visible.find(objid) != objects.visible.end()){
    ObjectView* obj = objects.cache[objid];
    if(obj == NULL){
      obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, objid);
      if(obj != NULL){
        objects.cache[objid] = obj;
      }
    }
    return obj;
  }else{
    return NULL;
  }
}

void PlayerView::updateObjectView(uint32_t objid){
  ObjectView* obj = objects.cache[objid];
  Game::getGame()->getPersistence()->saveObjectView(pid, obj);
}

void PlayerView::removeVisibleObject(uint32_t objid){
  ObjectView* obj = getObjectView(objid);
  if(obj != NULL){
    objects.cache[objid]->setGone(true);
    Game::getGame()->getPersistence()->saveObjectView(pid, objects.cache[objid]);
  }
}

bool PlayerView::isVisibleObject(uint32_t objid){
  if(objects.visible.find(objid) != objects.visible.end()){
    ObjectView* obj = objects.cache[objid];
    if(obj == NULL){
      obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, objid);
      if(obj != NULL){
        objects.cache[objid] = obj;
      }
    }
    return !(obj->isGone());
  }
  return false;
}

std::set<uint32_t> PlayerView::getVisibleObjects() const{
  return objects.visible;
}

void PlayerView::addOwnedObject(uint32_t objid){
  objects.actable.insert(objid);
  ObjectView* obj = getObjectView(objid);
  if(obj != NULL){
    obj->setCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveObjectView(pid, objects.cache[objid]);
  }else{
    ObjectView* ov = new ObjectView();
    ov->setObjectId(objid);
    ov->setCompletelyVisible(true);
    addVisibleObject(ov);
  }
}

void PlayerView::removeOwnedObject(uint32_t objid){
  objects.actable.erase(objid);
}

uint32_t PlayerView::getNumberOwnedObjects() const{
  return objects.actable.size();
}

std::set<uint32_t> PlayerView::getOwnedObjects() const{
  return objects.actable;
}

void PlayerView::processGetObject(uint32_t objid, Frame* frame){
  if(objects.visible.find(objid) == objects.visible.end()){
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
  
  if(seqnum != objects.sequence && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    objects.modified.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    objects.modified.clear();
    for(std::set<uint32_t>::iterator itcurr = objects.visible.begin();
        itcurr != objects.visible.end(); ++itcurr){
      ObjectView* obj = objects.cache[*itcurr];
      if(obj == NULL){
        obj = Game::getGame()->getPersistence()->retrieveObjectView(pid, *itcurr);
        if(obj != NULL){
          objects.cache[*itcurr] = obj;
        }
      }
      if((fromtime == UINT64_NEG_ONE && !(obj->isGone())) || obj->getModTime() > fromtime){
        objects.modified[*itcurr] = obj->getModTime();
      }
    }
  }
  
  if(snum > objects.modified.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, objects.modified.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > objects.modified.size() - snum){
    numtoget = objects.modified.size() - snum;
  }
    
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
    
  out->setType(ft03_ObjectIds_List);
  out->packInt(objects.sequence);
  out->packInt(objects.modified.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = objects.modified.begin();
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
  designs.visible.insert(designid);
  if(designs.cache.find(designid) != designs.cache.end()){
    if(designs.cache[designid] != NULL){
      delete designs.cache[designid];
    }
  }
  designs.cache[designid] = design;
  Game::getGame()->getPersistence()->saveDesignView(pid, design);
  designs.sequence++;
}

void PlayerView::addUsableDesign(uint32_t designid){
  designs.actable.insert(designid);
  if(designs.visible.find(designid) == designs.visible.end()){
    DesignView* designview = new DesignView();
    designview->setDesignId(designid);
    designview->setIsCompletelyVisible(true);
    addVisibleDesign(designview);
  }else{
    DesignView* design = designs.cache[designid];
    if(design == NULL){
      design = Game::getGame()->getPersistence()->retrieveDesignView(pid, designid);
      if(design != NULL){
        designs.cache[designid] = design;
      }
    }
    design->setIsCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveDesignView(pid, design);
    components.sequence++;
  }
}

void PlayerView::removeUsableDesign(uint32_t designid){
  std::set<uint32_t>::iterator dicurr = designs.actable.find(designid);
  if(dicurr != designs.actable.end())
    designs.actable.erase(dicurr);
}

bool PlayerView::isUsableDesign(uint32_t designid) const{
  return (designs.actable.find(designid) != designs.actable.end());
}

std::set<uint32_t> PlayerView::getUsableDesigns() const{
  return designs.actable;
}

std::set<uint32_t> PlayerView::getVisibleDesigns() const{
  return designs.visible;
}

void PlayerView::processGetDesign(uint32_t designid, Frame* frame){
  if(components.visible.find(designid) == designs.visible.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Design");
  }else{
    DesignView* design = designs.cache.find(designid)->second;
    if(design == NULL){
      design = Game::getGame()->getPersistence()->retrieveDesignView(pid, designid);
      if(design != NULL){
        designs.cache[designid] = design;
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
  
  if(seqnum != designs.sequence && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    designs.modified.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    designs.modified.clear();
    for(std::set<uint32_t>::iterator itcurr = designs.visible.begin();
        itcurr != designs.visible.end(); ++itcurr){
      DesignView* designv = designs.cache[*itcurr];
      if(designv == NULL){
        designv = Game::getGame()->getPersistence()->retrieveDesignView(pid, *itcurr);
        if(designv != NULL){
          designs.cache[*itcurr] = designv;
        }
      }
      if(fromtime == UINT64_NEG_ONE || designv->getModTime() > fromtime){
        designs.modified[*itcurr] = designv->getModTime();
      }
    }
  }
  
  if(snum > designs.modified.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, designs.visible.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > designs.modified.size() - snum){
    numtoget = designs.modified.size() - snum;
  }
    
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
    
  out->setType(ft03_DesignIds_List);
  out->packInt(designs.sequence);
  out->packInt(designs.modified.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = designs.modified.begin();
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
  components.visible.insert(compid);
  if(components.cache.find(compid) != components.cache.end()){
    if(components.cache[compid] != NULL){
      delete components.cache[compid];
    }
  }
  components.cache[compid] = comp;
  Game::getGame()->getPersistence()->saveComponentView(pid, comp);
  components.sequence++;
}

void PlayerView::addUsableComponent(uint32_t compid){
  components.actable.insert(compid);
  if(components.visible.find(compid) == components.visible.end()){
    ComponentView* compview = new ComponentView();
    compview->setComponentId(compid);
    compview->setCompletelyVisible(true);
    addVisibleComponent(compview);
  }else{
    ComponentView* compv = components.cache[compid];
    if(compv == NULL){
      compv = Game::getGame()->getPersistence()->retrieveComponentView(pid, compid);
      if(compv != NULL){
        components.cache[compid] = compv;
      }
    }
    compv->setCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveComponentView(pid, compv);
    components.sequence++;
  }
}

void PlayerView::removeUsableComponent(uint32_t compid){
  std::set<uint32_t>::iterator cicurr = components.actable.find(compid);
  if(cicurr != components.actable.end())
    components.actable.erase(cicurr);
}

bool PlayerView::isUsableComponent(uint32_t compid) const{
  return (components.actable.find(compid) != components.actable.end());
}

std::set<uint32_t> PlayerView::getVisibleComponents() const{
  return components.visible;
}

std::set<uint32_t> PlayerView::getUsableComponents() const{
  return components.actable;
}

void PlayerView::processGetComponent(uint32_t compid, Frame* frame){
  if(components.visible.find(compid) == components.visible.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Component");
  }else{
    ComponentView* component = components.cache.find(compid)->second;
    if(component == NULL){
      component = Game::getGame()->getPersistence()->retrieveComponentView(pid, compid);
      if(component != NULL){
        components.cache[compid] = component;
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
  
  if(seqnum != components.sequence && seqnum != UINT32_NEG_ONE){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    components.modified.clear();
    return;
  }
  
  if(seqnum == UINT32_NEG_ONE){
    //clear current mod list in case it has stuff in it still
    components.modified.clear();
    for(std::set<uint32_t>::iterator itcurr = components.visible.begin();
        itcurr != components.visible.end(); ++itcurr){
      ComponentView* component = components.cache[*itcurr];
      if(component == NULL){
        component = Game::getGame()->getPersistence()->retrieveComponentView(pid, *itcurr);
        if(component != NULL){
          components.cache[*itcurr] = component;
        }
      }
      if(fromtime == UINT64_NEG_ONE || component->getModTime() > fromtime){
        components.modified[*itcurr] = component->getModTime();
      }
    }
  }
  
  if(snum > components.modified.size()){
    Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, components.modified.size());
    out->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > components.modified.size() - snum){
    numtoget = components.modified.size() - snum;
  }
  
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    Logger::getLogger()->debug("Number of items to get too high, numtoget = %d", numtoget);
    out->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
  
  out->setType(ft03_ComponentIds_List);
  out->packInt(components.sequence);
  out->packInt(components.modified.size() - snum - numtoget);
  out->packInt(numtoget);
  std::map<uint32_t, uint64_t>::iterator itcurr = components.modified.begin();
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
  objects.visible = obids;
}

void PlayerView::setOwnedObjects(const std::set<uint32_t>& obids){
  objects.actable = obids;
}

void PlayerView::setVisibleDesigns(const std::set<uint32_t>& dids){
  designs.visible = dids;
}

void PlayerView::setUsableDesigns(const std::set<uint32_t>& dids){
  designs.actable = dids;
}

void PlayerView::setVisibleComponents(const std::set<uint32_t>& cids){
  components.visible = cids;
}

void PlayerView::setUsableComponents(const std::set<uint32_t>& cids){
  components.actable = cids;
}
