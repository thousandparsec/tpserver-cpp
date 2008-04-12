/*  DesignView class
 *
 *  Copyright (C) 2005,2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include "designstore.h"
#include "design.h"

#include "designview.h"

DesignView::DesignView() : designid(0), completelyvisible(false), seename(false), name(), seedesc(false),
    description(), seenum(false), exist(0), seeowner(false), owner(0), components(), properties(){
  timestamp = time(NULL);
}

DesignView::~DesignView(){

}

void DesignView::packFrame(Frame* frame) const{
  
  Design* design = Game::getGame()->getDesignStore()->getDesign(designid);
  
  frame->setType(ft03_Design);
  frame->packInt(designid);
  if(completelyvisible && timestamp < design->getModTime()){
    frame->packInt64(design->getModTime());
  }else{
    frame->packInt64(timestamp);
  }
  frame->packInt(1);
  frame->packInt(design->getCategoryId());
  if(completelyvisible || seename){
    frame->packString(design->getName());
  }else{
    frame->packString(name);
  }
  if(completelyvisible || seedesc){
    frame->packString(design->getDescription());
  }else{
    frame->packString(description);
  }
  if(completelyvisible || seenum){
    frame->packInt(design->isValid() ? design->getInUse() : UINT32_NEG_ONE);
  }else{
    frame->packInt(exist);
  }
  if(completelyvisible || seeowner){
    frame->packInt(design->getOwner());
  }else{
    frame->packInt(owner);
  }
  std::map<uint32_t, uint32_t> complist;
  if(completelyvisible){
    complist = design->getComponents();
  }else{
    complist = components;
  }
  frame->packInt(complist.size());
  for(std::map<uint32_t, uint32_t>::const_iterator itcurr = complist.begin();
      itcurr != complist.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packInt(itcurr->second);
  }
  if(completelyvisible){
    frame->packString(design->getFeedback());
  }else{
    frame->packString("");
  }
  std::map<uint32_t, PropertyValue> proplist;
  if(completelyvisible){
    proplist = design->getPropertyValues();
  }else{
    proplist = properties;
  }
  frame->packInt(proplist.size());
  for(std::map<uint32_t, PropertyValue>::const_iterator itcurr = proplist.begin();
      itcurr != proplist.end(); ++itcurr){
    itcurr->second.packFrame(frame);
  }
}

uint32_t DesignView::getDesignId() const{
  return designid;
}

bool DesignView::isCompletelyVisible() const{
  return completelyvisible;
}

std::string DesignView::getVisibleName() const{
  return name;
}

bool DesignView::canSeeName() const{
  return seename;
}

std::string DesignView::getVisibleDescription() const{
    return description;
}

bool DesignView::canSeeDescription() const{
  return seedesc;
}

uint32_t DesignView::getVisibleOwner() const{
  return owner;
}

bool DesignView::canSeeOwner() const{
  return seeowner;
}

std::map<uint32_t, uint32_t> DesignView::getVisibleComponents() const{
  return components;
}

uint32_t DesignView::getVisibleNumExist() const{
  return exist;
}

bool DesignView::canSeeNumExist() const{
  return seenum;
}

std::map<uint32_t, PropertyValue> DesignView::getVisiblePropertyValues() const{
    return properties;
}

uint64_t DesignView::getModTime() const{
    return timestamp;
}

void DesignView::setDesignId(uint32_t id){
  designid = id;
  touchModTime();
}


void DesignView::setIsCompletelyVisible(bool ncv){
  completelyvisible = ncv;
  touchModTime();
}

void DesignView::setVisibleName(const std::string& n){
  name = n;
  touchModTime();
}

void DesignView::setCanSeeName(bool csn){
  seename = csn;
  touchModTime();
}

void DesignView::setVisibleDescription(const std::string& d){
  description = d;
  touchModTime();
}

void DesignView::setCanSeeDescription(bool csd){
  seedesc = csd;
  touchModTime();
}

void DesignView::setVisibleOwner(uint32_t o){
  owner = o;
  touchModTime();
}

void DesignView::setCanSeeOwner(bool cso){
  seeowner = cso;
  touchModTime();
}

void DesignView::setVisibleComponents(std::map<uint32_t, uint32_t> cl){
  components = cl;
  touchModTime();
}

void DesignView::setVisibleNumExist(uint32_t nne){
    exist = nne;
    touchModTime();
}

void DesignView::setCanSeeNumExist(bool csn){
  seenum = csn;
  touchModTime();
}

void DesignView::setModTime(uint64_t nmt){
    timestamp = nmt;
}

void DesignView::setVisiblePropertyValues(std::map<uint32_t, PropertyValue> pvl){
  properties = pvl;
  touchModTime();
}

void DesignView::touchModTime(){
    timestamp = time(NULL);
}
