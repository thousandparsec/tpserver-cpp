/*  ObjectData base class
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include "order.h"
#include "objectparameter.h"
#include "objectparametergroup.h"

#include "objectdata.h"

ObjectData::ObjectData(){
  touchModTime();
}

ObjectData::~ObjectData(){
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    delete (*itcurr);
  }
}

void ObjectData::packObjectParameters(Frame* frame, uint32_t playerid){
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    (*itcurr)->packObjectFrame(frame, playerid);
  }
}

bool ObjectData::unpackModifyObject(Frame* frame, uint32_t playerid){
  bool rtn = true;
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    rtn = rtn & (*itcurr)->unpackModifyObjectFrame(frame, playerid);
    if(!rtn)
      break;
  }
  return rtn;
}

void ObjectData::packObjectDescFrame(Frame* frame){
  frame->packString(nametype);
  frame->packString(typedesc);
  frame->packInt64(modtime);
  frame->packInt(paramgroups.size());
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    (*itcurr)->packObjectDescFrame(frame);
  }
}

ObjectParameter* ObjectData::getParameterByType(uint32_t ptype){
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    std::list<ObjectParameter*> params = (*itcurr)->getParameters();
    for(std::list<ObjectParameter*>::iterator opitcurr = params.begin(); opitcurr != params.end();
        ++opitcurr){
      if((*opitcurr)->getType() == ptype){
        return (*opitcurr);
      }
    }
  }
  return NULL;
}

void ObjectData::touchModTime(){
  modtime = time(NULL);
}

long long ObjectData::getModTime() const{
  return modtime;
}

void ObjectData::setModTime(uint64_t time){
    modtime = time;
}

void ObjectData::signalRemoval(){
  for(std::list<ObjectParameterGroup*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    (*itcurr)->signalRemoval();
  }
}
