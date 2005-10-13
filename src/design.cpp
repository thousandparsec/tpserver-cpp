/*  Design class
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
#include "tpscheme.h"

#include "design.h"

Design::Design(){
  valid = false;
}

Design::~Design(){

}

void Design::packFrame(Frame* frame) const{
  frame->setType(ft03_Design);
  frame->packInt(designid);
  frame->packInt64(timestamp);
  frame->packInt(1);
  frame->packInt(catid);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packInt(valid ? inuse : 0xFFFFFFFF);
  frame->packInt(owner);
  frame->packInt(components.size());
  for(std::map<unsigned int, unsigned int>::const_iterator itcurr = components.begin();
      itcurr != components.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packInt(itcurr->second);
  }
  frame->packString(feedback.c_str());
  frame->packInt(properties.size());
  for(std::map<unsigned int, PropertyValue>::const_iterator itcurr = properties.begin();
      itcurr != properties.end(); ++itcurr){
    itcurr->second.packFrame(frame);
  }
}

unsigned int Design::getDesignId() const{
  return designid;
}

unsigned int Design::getCategoryId() const{
  return catid;
}

std::string Design::getName() const{
  return name;
}

unsigned int Design::getOwner() const{
  return owner;
}

std::map<unsigned int, unsigned int> Design::getComponents() const{
  return components;
}

unsigned int Design::getNumExist() const{
  return exist;
}

bool Design::isValid() const{
  return valid;
}

double Design::getPropertyValue(unsigned int propid) const{
  std::map<unsigned int, PropertyValue>::const_iterator itpos = properties.find(propid);
  if(itpos != properties.end()){
    PropertyValue pv = itpos->second;
    return pv.getValue();
  }else
    return 0.0;
}

void Design::setDesignId(unsigned int id){
  designid = id;
}

void Design::setCategoryId(unsigned int id){
  catid = id;
}

void Design::setName(const std::string& n){
  name = n;
}

void Design::setDescription(const std::string& d){
  description = d;
}

void Design::setOwner(unsigned int o){
  owner = o;
}

void Design::setComponents(std::map<unsigned int, unsigned int> cl){
  components = cl;
  valid = false;
  properties.clear();
  feedback = "";
}

void Design::eval(){
  // calculate design property values
  
  //first, clear out any old ones
  valid = false;
  properties.clear();
  feedback = "";

  // start calc
  TpScheme* scheme_intr = TpScheme::getImplemention();
  scheme_intr->evalDesign(this);
}

void Design::setPropertyValues(std::map<unsigned int, PropertyValue> pvl){
  properties = pvl;
}

void Design::setValid(bool v, const std::string& f){
  valid = v;
  feedback = f;
}
