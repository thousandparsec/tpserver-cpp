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
#include "algorithms.h"
#include <boost/bind.hpp>

ObjectParameterGroup::ObjectParameterGroup() : groupid(0), parameters(){
}

ObjectParameterGroup::ObjectParameterGroup(const ObjectParameterGroup &rhs): parameters(){
  groupid = rhs.groupid;
  // TODO: stl-ify
  for(ParameterList::const_iterator itcurr = rhs.parameters.begin();
      itcurr != rhs.parameters.end(); ++itcurr){
    parameters.push_back((*itcurr)->clone());
  }
}

ObjectParameterGroup::~ObjectParameterGroup(){
  delete_all( parameters );
  for(ParameterList::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    delete *itcurr;
  }
}

uint32_t ObjectParameterGroup::getGroupId() const{
  return groupid;
}

ObjectParameterGroup::ParameterList ObjectParameterGroup::getParameters() const{
  return parameters;
}

ObjectParameter* ObjectParameterGroup::getParameter(uint32_t paramid) const{
  if(paramid < parameters.size() + 1 && paramid != 0){
    return parameters[--paramid];
  }
  return NULL;
}

void ObjectParameterGroup::setGroupId(uint32_t ni){
  groupid = ni;
}

void ObjectParameterGroup::addParameter(ObjectParameter* op){
  parameters.push_back(op);
}

void ObjectParameterGroup::packObjectFrame(Frame * f, uint32_t playerid){
  std::for_each( parameters.begin(), parameters.end(), boost::bind( &ObjectParameter::packObjectFrame, _1, f, playerid ) );
}

bool ObjectParameterGroup::unpackModifyObjectFrame(Frame * f, uint32_t playerid){
  bool rtn = true;
  // TODO: exceptions then for_each
  for(ParameterList::iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    rtn = rtn & (*itcurr)->unpackModifyObjectFrame(f, playerid);
    if(!rtn)
      break;
  }
  return rtn;
}

void ObjectParameterGroup::signalRemoval(){
  std::for_each( parameters.begin(), parameters.end(), boost::mem_fn( &ObjectParameter::signalRemoval ) );
}
