/*  ComponentView class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include "component.h"
#include "designstore.h"

#include "componentview.h"

ComponentView::ComponentView(): compid(0), completelyvisible(false), visiblecats(), seename(false),
                              visiblename(), seedesc(false), visibledesc(), seerequirements(false),
                              visibleproperties(){
}

ComponentView::~ComponentView(){

}

void ComponentView::packFrame(Frame* frame) const{
  
  Component* comp = Game::getGame()->getDesignStore()->getComponent(compid);
  
  frame->setType(ft03_Component);
  frame->packInt(compid);
  if(completelyvisible && getModTime() < comp->getModTime()){
    frame->packInt64(comp->getModTime());
  }else{
    frame->packInt64(getModTime());
  }
  std::set<uint32_t> catids;
  if(completelyvisible){
    catids = comp->getCategoryIds();
  }else{
    catids = visiblecats;
  }
  //Maybe: filter catids list for non-visible categories?
  frame->packInt(catids.size());
  for(std::set<uint32_t>::const_iterator idit = catids.begin(); idit != catids.end(); ++idit){
    frame->packInt(*idit);
  }
  if(completelyvisible || seename){
    frame->packString(comp->getName());
  }else{
    frame->packString(visiblename);
  }
  if(completelyvisible || seedesc){
    frame->packString(comp->getDescription());
  }else{
    frame->packString(visibledesc);
  }
  if(completelyvisible || seerequirements){
    frame->packString(comp->getTpclRequirementsFunction());
  }else{
    frame->packString("(lamda (design) (cons #f \"Unknown Component. \")))");
  }
  std::map<uint32_t, std::string> propertylist;
  if(completelyvisible){
    propertylist = comp->getPropertyList();
  }else{
    std::map<uint32_t, std::string> compproplist = comp->getPropertyList();
    for(std::set<uint32_t>::const_iterator itcurr = visibleproperties.begin();
        itcurr != visibleproperties.end(); ++itcurr){
      propertylist[*itcurr] = compproplist[*itcurr];
    }
  }
  frame->packInt(propertylist.size());
  for(std::map<uint32_t, std::string>::const_iterator itcurr = propertylist.begin(); itcurr != propertylist.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packString(itcurr->second);
  }
}

uint32_t ComponentView::getComponentId() const{
  return compid;
}

bool ComponentView::isCompletelyVisible() const{
  return completelyvisible;
}

std::set<uint32_t> ComponentView::getVisibleCategories() const{
  //maybe: filter out non-visible cats
  return visiblecats;
}

bool ComponentView::canSeeName() const{
  return seename;
}

std::string ComponentView::getVisibleName() const{
  return visiblename;
}

bool ComponentView::canSeeDescription() const{
  return seedesc;
}

std::string ComponentView::getVisibleDescription() const{
    return visibledesc;
}

bool ComponentView::canSeeRequirementsFunc() const{
  return seerequirements;
}


std::set<uint32_t> ComponentView::getVisiblePropertyFuncs() const{
  return visibleproperties;
}

void ComponentView::setComponentId(uint32_t id){
  compid = id;
  touchModTime();
}

void ComponentView::setCompletelyVisible(bool ncv){
  if(ncv != completelyvisible)
    touchModTime();
  completelyvisible = ncv;
}

void ComponentView::setVisibleCategories(const std::set<uint32_t>& nvc){
  visiblecats = nvc;
  touchModTime();
}

void ComponentView::addVisibleCategory(uint32_t catid){
  visiblecats.insert(catid);
  touchModTime();
}

void ComponentView::setCanSeeName(bool csn){
  if(csn != seename)
    touchModTime();
  seename = csn;
}

void ComponentView::setVisibleName(const std::string& nvn){
  visiblename = nvn;
  touchModTime();
}

void ComponentView::setCanSeeDescription(bool csd){
  if(csd != seedesc)
    touchModTime();
  seedesc = csd;
}

void ComponentView::setVisibleDescription(const std::string& nvd){
  visibledesc = nvd;
  touchModTime();
}

void ComponentView::setCanSeeRequirementsFunc(bool csr){
  if(csr != seerequirements)
    touchModTime();
  seerequirements = csr;
}

void ComponentView::setVisiblePropertyFuncs(const std::set<uint32_t>& nvp){
  visibleproperties = nvp;
  touchModTime();
}

void ComponentView::addVisiblePropertyFunc(uint32_t propid){
  visibleproperties.insert(propid);
  touchModTime();
}

