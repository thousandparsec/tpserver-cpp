/*  Component class
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
#include "designstore.h"
#include "design.h"

#include "component.h"

Component::Component(): catids(), inuse(false), parentdesignid(0){
  compid = 0;
    timestamp = time(NULL);
}

Component::~Component(){

}

void Component::packFrame(Frame* frame) const{
  frame->setType(ft03_Component);
  frame->packInt(compid);
  frame->packInt64(timestamp);
  frame->packInt(catids.size());
  for(std::set<uint32_t>::const_iterator idit = catids.begin(); idit != catids.end(); ++idit){
    frame->packInt(*idit);
  }
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packString(tpcl_requirements.c_str());
  frame->packInt(propertylist.size());
  for(std::map<unsigned int, std::string>::const_iterator itcurr = propertylist.begin(); itcurr != propertylist.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packString(itcurr->second.c_str());
  }
}

Component* Component::copy() const{
  Component * comp = new Component();
  comp->compid = compid;
  comp->catids = catids;
  comp->timestamp = timestamp;
  comp->name = name;
  comp->description = description;
  comp->tpcl_requirements = tpcl_requirements;
  comp->propertylist = propertylist;
  return comp;
}

unsigned int Component::getComponentId() const{
  return compid;
}

std::set<uint32_t> Component::getCategoryIds() const{
  return catids;
}

bool Component::isInCategory(uint32_t id) const{
  return catids.count(id) != 0;
}

std::string Component::getName() const{
  return name;
}

std::string Component::getDescription() const{
    return description;
}

std::string Component::getTpclRequirementsFunction() const{
  return tpcl_requirements;
}

std::map<unsigned int, std::string> Component::getPropertyList() const{
  return propertylist;
}

uint64_t Component::getModTime() const{
    return timestamp;
}

void Component::setComponentId(unsigned int id){
  compid = id;
}

void Component::setCategoryIds(const std::set<uint32_t>& ids){
  catids = ids;
}

void Component::addCategoryId(uint32_t id){
  catids.insert(id);
}

void Component::setName(const std::string& n){
  name = n;
}

void Component::setDescription(const std::string& d){
  description = d;
}

void Component::setTpclRequirementsFunction(const std::string& a){
  tpcl_requirements = a;
}

void Component::setPropertyList(std::map<unsigned int, std::string> pl){
  propertylist = pl;
}

void Component::setModTime(uint64_t nmt){
    timestamp = nmt;
}

void Component::setInUse(bool used){
  inuse = used;
  if(parentdesignid != 0){
    DesignStore* ds = Game::getGame()->getDesignStore();
    Design* design = ds->getDesign(parentdesignid);
    if(used){
      design->addUnderConstruction(1);
    }else{
      design->removeCanceledConstruction(1);
    }
    ds->designCountsUpdated(design);
  }
}

bool Component::isInUse() const{
  return inuse;
}

void Component::setParentDesignId(uint32_t designid){
  parentdesignid = designid;
}

uint32_t Component::getParentDesignId() const{
  return parentdesignid;
}
