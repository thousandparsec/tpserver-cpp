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
  turnCompdifflist[compid] = ModListItem(compid, time(NULL));
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

uint32_t PlayerView::getObjectSequenceKey() const{
  return currObjSeq;
}
