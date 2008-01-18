/*  ObjectType base class
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include "object.h"
#include "objectparametergroupdesc.h"
#include "objectbehaviour.h"

#include "objecttype.h"

ObjectType::ObjectType() : nextparamgroupid(1){
  touchModTime();
}

ObjectType::~ObjectType(){
  for(std::map<uint32_t, ObjectParameterGroupDesc*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    delete (itcurr->second);
  }
}

std::string ObjectType::getTypeName() const{
  return nametype;
}

long long ObjectType::getModTime() const{
  return modtime;
}


void ObjectType::packObjectDescFrame(Frame* frame){
  frame->packString(nametype);
  frame->packString(typedesc);
  frame->packInt64(modtime);
  frame->packInt(paramgroups.size());
  for(std::map<uint32_t, ObjectParameterGroupDesc*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    (itcurr->second)->packObjectDescFrame(frame);
  }
}

void ObjectType::setupObject(IGObject* obj) const{
  for(std::map<uint32_t, ObjectParameterGroupDesc*>::const_iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    obj->setParameterGroup((itcurr->second)->createObjectParameterGroup());
  }
  ObjectBehaviour* ob = createObjectBehaviour();
  ob->setObject(obj);
  obj->setObjectBehaviour(ob);
  ob->setupObject();
}

void ObjectType::addParameterGroupDesc(ObjectParameterGroupDesc* group){
  group->setGroupId(nextparamgroupid);
  paramgroups[nextparamgroupid] = group;
  nextparamgroupid++;
  touchModTime();
}

ObjectParameterGroupDesc* ObjectType::getParameterGroupDesc(uint32_t groupid) const{
  if(paramgroups.find(groupid) != paramgroups.end()){
    return paramgroups.find(groupid)->second;
  }
  return NULL;
}

void ObjectType::touchModTime(){
  modtime = time(NULL);
}

