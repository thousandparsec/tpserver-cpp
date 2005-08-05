/*  Component class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"

#include "component.h"

Component::Component(){
  compid = 0;
}

Component::~Component(){

}

void Component::packFrame(Frame* frame) const{
  frame->setType(ft03_Component);
  frame->packInt(compid);
  frame->packInt64(timestamp);
  frame->packInt(1);
  frame->packInt(catid);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packString(tpcl_requirements.c_str());
  frame->packInt(propertylist.size());
  for(std::map<unsigned int, std::string>::const_iterator itcurr = propertylist.begin(); itcurr != propertylist.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packString(itcurr->second.c_str());
  }
}

unsigned int Component::getComponentId() const{
  return compid;
}

unsigned int Component::getCategoryId() const{
  return catid;
}

std::string Component::getName() const{
  return name;
}

std::string Component::getTpclRequirementsFunction() const{
  return tpcl_requirements;
}

std::map<unsigned int, std::string> Component::getPropertyList() const{
  return propertylist;
}

void Component::setComponentId(unsigned int id){
  compid = id;
}

void Component::setCategoryId(unsigned int id){
  catid = id;
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
