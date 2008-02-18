/*  ObjectParameterGroup class
 *
 *  Copyright (C) 2007, 2008 Lee Begg and the Thousand Parsec Project
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

ObjectParameterGroupData::ObjectParameterGroupData() : groupid(0), parameters(){
}

ObjectParameterGroupData::ObjectParameterGroupData(const ObjectParameterGroupData &rhs): parameters(){
  groupid = rhs.groupid;
  for(std::list<ObjectParameter*>::const_iterator itcurr = rhs.parameters.begin();
      itcurr != rhs.parameters.end(); ++itcurr){
    parameters.push_back((*itcurr)->clone());
  }
}

ObjectParameterGroupData::~ObjectParameterGroupData(){
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    delete *itcurr;
  }
}

uint32_t ObjectParameterGroupData::getGroupId() const{
  return groupid;
}

std::list<ObjectParameter*> ObjectParameterGroupData::getParameters() const{
  return parameters;
}

ObjectParameter* ObjectParameterGroupData::getParameter(uint32_t paramid) const{
  if(paramid < parameters.size() + 1 && paramid != 0){
    paramid--;
    std::list<ObjectParameter*>::const_iterator itcurr = parameters.begin();
    advance(itcurr, paramid);
    return (*itcurr);
  }
  return NULL;
}

void ObjectParameterGroupData::setGroupId(uint32_t ni){
  groupid = ni;
}

void ObjectParameterGroupData::addParameter(ObjectParameter* op){
  parameters.push_back(op);
}

void ObjectParameterGroupData::packObjectFrame(Frame * f, uint32_t playerid){
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr)->packObjectFrame(f, playerid);
  }
}

bool ObjectParameterGroupData::unpackModifyObjectFrame(Frame * f, unsigned int playerid){
  bool rtn = true;
  for(std::list<ObjectParameter*>::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    rtn = rtn & (*itcurr)->unpackModifyObjectFrame(f, playerid);
    if(!rtn)
      break;
  }
  return rtn;
}

void ObjectParameterGroupData::signalRemoval(){
  for(std::list<ObjectParameter*>::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr)->signalRemoval();
  }
}
