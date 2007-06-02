/*  PlayerView object, holding the player's view of the universe
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include "designstore.h"
#include "design.h"
#include "component.h"

#include "playerview.h"

ModListItem::ModListItem() : id(0), modtime(0){
}

ModListItem::ModListItem(uint32_t nid, uint64_t nmt) : id(nid), modtime(nmt){
}

PlayerView::PlayerView() : pid(0), visibleObjects(), currObjSeq(0), visibleDesigns(), usableDesigns(),
    visibleComponents(), usableComponents(), cacheComponents(),
    difflistComponents(), turnCompdifflist(), currCompSeq(0){
}

PlayerView::~PlayerView(){
}

void PlayerView::setPlayerId(uint32_t newid){
  pid = newid;
}

void PlayerView::doOnceATurn(){
  DesignStore* ds = Game::getGame()->getDesignStore();
  
  //Components
  // check for updated usable components
  for(std::set<uint32_t>::iterator itcurr = usableComponents.begin();
      itcurr != usableComponents.end(); ++itcurr){
    Component* comp = ds->getComponent(*itcurr);
    if(comp->getModTime() > cacheComponents[*itcurr]->getModTime()){
      addVisibleComponent(comp->copy());
    }
  }
  // build new diff list
  for(std::map<uint32_t, ModListItem>::iterator itcurr = turnCompdifflist.begin();
      itcurr != turnCompdifflist.end(); ++itcurr){
    difflistComponents.push_back(itcurr->second);
  }
  turnCompdifflist.clear();
}

void PlayerView::setVisibleObjects(std::set<unsigned int> vis){
  visibleObjects = vis;
  currObjSeq++;
}

bool PlayerView::isVisibleObject(unsigned int objid){
  return visibleObjects.find(objid) != visibleObjects.end();
}

std::set<uint32_t> PlayerView::getVisibleObjects() const{
  return visibleObjects;
}

void PlayerView::addVisibleDesign(unsigned int designid){
  visibleDesigns.insert(designid);
}

void PlayerView::addUsableDesign(unsigned int designid){
  usableDesigns.insert(designid);
  Logger::getLogger()->debug("Added valid design");
}

void PlayerView::removeUsableDesign(unsigned int designid){
  std::set<unsigned int>::iterator dicurr = usableDesigns.find(designid);
  if(dicurr != usableDesigns.end())
    usableDesigns.erase(dicurr);
}

bool PlayerView::isUsableDesign(unsigned int designid) const{
  return (usableDesigns.find(designid) != usableDesigns.end());
}

std::set<unsigned int> PlayerView::getUsableDesigns() const{
  return usableDesigns;
}

std::set<uint32_t> PlayerView::getVisibleDesigns() const{
  return visibleDesigns;
}

void PlayerView::addVisibleComponent(Component* comp){
  comp->setModTime(time(NULL));
  uint32_t compid = comp->getComponentId();
  visibleComponents.insert(compid);
  if(cacheComponents.find(compid) != cacheComponents.end()){
    delete cacheComponents[compid];
  }
  cacheComponents[compid] = comp;
  currCompSeq++;
  turnCompdifflist[compid] = ModListItem(compid, comp->getModTime());
}

void PlayerView::addUsableComponent(uint32_t compid){
  usableComponents.insert(compid);
  if(visibleComponents.find(compid) == visibleComponents.end()){
    addVisibleComponent(Game::getGame()->getDesignStore()->getComponent(compid)->copy());
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

void PlayerView::processGetComponent(uint32_t compid, Frame* frame) const{
  if(visibleComponents.find(compid) == visibleComponents.end()){
    frame->createFailFrame(fec_NonExistant, "No Such Component");
  }else{
    Component* component = cacheComponents.find(compid)->second;
    component->packFrame(frame);
  }
}

void PlayerView::processGetComponentIds(Frame* in, Frame* out) const{
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
  uint64_t fromtime = 0xffffffffffffffffULL;
  if(in->getVersion() >= fv0_4){
    fromtime = in->unpackInt64();
  }
  
  if(seqnum != currCompSeq && seqnum != 0xffffffff){
    out->createFailFrame(fec_FrameError, "Invalid Sequence number");
    return;
  }
  
  
  
  if(fromtime == 0xffffffffffffffffULL){
  
    if(snum > visibleComponents.size()){
      Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, visibleComponents.size());
      out->createFailFrame(fec_NonExistant, "Starting number too high");
      return;
    }
    if(numtoget > visibleComponents.size() - snum){
      numtoget = visibleComponents.size() - snum;
    }
    
    out->setType(ft03_ComponentIds_List);
    out->packInt(currCompSeq);
    out->packInt(visibleComponents.size() - snum - numtoget);
    out->packInt(numtoget);
    std::set<uint32_t>::iterator itcurr = visibleComponents.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
      out->packInt(*itcurr);
      out->packInt64(cacheComponents.find(*itcurr)->second->getModTime());
    }
    
    if(out->getVersion() >= fv0_4){
      out->packInt64(fromtime);
    }
    
  }else{
    
    out->setType(ft03_ComponentIds_List);
    out->packInt(currCompSeq);
    
    // get list of changes since fromtime
    
    std::list<ModListItem> difflist;
    for(std::list<ModListItem>::const_iterator itcurr = difflistComponents.begin();
        itcurr != difflistComponents.end(); ++itcurr){
      if((*itcurr).modtime >= fromtime){
        difflist.insert(difflist.end(), itcurr, difflistComponents.end());
        break;
      }
    }
    
    if(snum > difflist.size()){
      Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, difflist.size());
      out->createFailFrame(fec_NonExistant, "Starting number too high");
      return;
    }
    if(numtoget > difflist.size() - snum){
      numtoget = difflist.size() - snum;
    }
    
    out->packInt(visibleComponents.size() - snum - numtoget);
    out->packInt(numtoget);
    
    std::list<ModListItem>::iterator itcurr = difflist.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
      out->packInt((*itcurr).id);
      out->packInt64((*itcurr).modtime);
    }
    
    out->packInt64(fromtime);
    
  }
}

uint32_t PlayerView::getObjectSequenceKey() const{
  return currObjSeq;
}
