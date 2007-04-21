/*  ObjectParameterGroup class
 *
 *  Copyright (C) 2007 Lee Begg and the Thousand Parsec Project
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

#include <stdlib.h>

#include "frame.h"
#include "objectparameter.h"

#include "objectparametergroup.h"

ObjectParameterGroup::ObjectParameterGroup() : groupid(0), name(), description(), parameters(){
}

ObjectParameterGroup::ObjectParameterGroup(const ObjectParameterGroup &rhs): parameters(){
  groupid = rhs.groupid;
  name = rhs.name;
  description = rhs.description;
  for(std::list<ObjectParameter*>::const_iterator itcurr = rhs.parameters.begin();
      itcurr != rhs.parameters.end(); ++itcurr){
    parameters.push_back((*itcurr)->clone());
  }
}

ObjectParameterGroup::~ObjectParameterGroup(){
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    delete *itcurr;
  }
}

uint32_t ObjectParameterGroup::getGroupId() const{
  return groupid;
}

std::string ObjectParameterGroup::getName() const{
  return name;
}

std::string ObjectParameterGroup::getDescription() const{
  return description;
}

std::list<ObjectParameter*> ObjectParameterGroup::getParameters() const{
  return parameters;
}

void ObjectParameterGroup::setGroupId(uint32_t ni){
  groupid = ni;
}

void ObjectParameterGroup::setName(const std::string& nn){
  name = nn;
}

void ObjectParameterGroup::setDescription(const std::string& nd){
  description = nd;
}

void ObjectParameterGroup::addParameter(ObjectParameter* op){
  parameters.push_back(op);
}

void ObjectParameterGroup::packObjectFrame(Frame * f, uint32_t playerid){
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr)->packObjectFrame(f, playerid);
  }
}

void ObjectParameterGroup::packObjectDescFrame(Frame * f) const{

  f->packInt(groupid);
  f->packString(name);
  f->packString(description);
  f->packInt(parameters.size());
  for(std::list<ObjectParameter*>::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr)->packObjectDescFrame(f);
  }
}

bool ObjectParameterGroup::unpackModifyObjectFrame(Frame * f, unsigned int playerid){
  bool rtn = true;
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    rtn = rtn & (*itcurr)->unpackModifyObjectFrame(f, playerid);
    if(!rtn)
      break;
  }
  return rtn;
}

void ObjectParameterGroup::signalRemoval(){
  for(std::list<ObjectParameter*>::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr)->signalRemoval();
  }
}
