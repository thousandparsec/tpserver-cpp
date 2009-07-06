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

#include "game.h"
#include "designstore.h"
#include "design.h"

#include "component.h"

Component::Component(): Describable(0), catids(), inuse(false), parentdesignid(0){
}

Component::~Component(){

}

uint32_t Component::getComponentId() const{
  return getId();
}

IdSet Component::getCategoryIds() const{
  return catids;
}

bool Component::isInCategory(uint32_t id) const{
  return catids.count(id) != 0;
}

std::string Component::getTpclRequirementsFunction() const{
  return tpcl_requirements;
}

std::map<uint32_t, std::string> Component::getPropertyList() const{
  return propertylist;
}

void Component::setComponentId(uint32_t nid){
  setId(nid);
}

void Component::setCategoryIds(const IdSet& ids){
  catids = ids;
}

void Component::addCategoryId(uint32_t id){
  catids.insert(id);
}

void Component::setTpclRequirementsFunction(const std::string& a){
  tpcl_requirements = a;
}

void Component::setPropertyList(std::map<uint32_t, std::string> pl){
  propertylist = pl;
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
