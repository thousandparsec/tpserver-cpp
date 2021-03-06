/*  ObjectBehaviour base class
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

#include "object.h"

#include "objectbehaviour.h"


ObjectBehaviour::ObjectBehaviour() : obj(NULL){
}

ObjectBehaviour::~ObjectBehaviour(){
}

void ObjectBehaviour::setObject(IGObject* nobj){
  obj = nobj;
}

void ObjectBehaviour::packExtraData(OutputFrame::Ptr frame){
}

void ObjectBehaviour::setupObject(){
}

void ObjectBehaviour::signalRemoval(){
  //for(std::map<uint32_t, ObjectParameterGroupDesc*>::iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
 //     ++itcurr){
//    (itcurr->second)->signalRemoval();
 // }
}

