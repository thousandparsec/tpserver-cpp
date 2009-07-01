/*  ObjectParameterGroup class
 *
 *  Copyright (C) 2007, 2008, 2009 Lee Begg and the Thousand Parsec Project
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
#include <exception>

#include "frame.h"
#include "position3dobjectparam.h"
#include "velocity3dobjectparam.h"
#include "sizeobjectparam.h"
#include "orderqueueobjectparam.h"
#include "resourcelistobjectparam.h"
#include "referenceobjectparam.h"
#include "refquantitylistobjectparam.h"
#include "integerobjectparam.h"
#include "mediaobjectparam.h"
#include "logging.h"

#include "objectparametergroupdesc.h"


ObjectParameterDesc::ObjectParameterDesc() : type(0), name(), description(){
}

ObjectParameterDesc::ObjectParameterDesc(const ObjectParameterDesc& rhs){
  type = rhs.type;
  name = rhs.name;
  description = rhs.description;
}

ObjectParameterDesc::~ObjectParameterDesc(){
}

ObjectParameterDesc& ObjectParameterDesc::operator=(const ObjectParameterDesc& rhs){
  type = rhs.type;
  name = rhs.name;
  description = rhs.description;
  return *this;
}

void ObjectParameterDesc::setType(uint32_t nt){
  type = nt;
}

void ObjectParameterDesc::setName(const std::string& nn){
  name = nn;
}

void ObjectParameterDesc::setDescription(const std::string& nd){
  description = nd;
}

uint32_t ObjectParameterDesc::getType() const{
  return type;
}

void ObjectParameterDesc::packObjectDescFrame(Frame* f)const {
  f->packString(name);
  f->packInt(type);
  f->packString(description);
  //HACK: probably should be made configurable
  // Maybe have subclasses of ObjectParamDesc, or add field of ObjectParamDesc.
  if(type == obpT_Order_Queue){
      f->packInt(100);
  }
}

ObjectParameterGroupDesc::ObjectParameterGroupDesc() : Describable(0), parameters(){
}

ObjectParameterGroupDesc::~ObjectParameterGroupDesc(){
}

uint32_t ObjectParameterGroupDesc::getGroupId() const{
  return getId();
}

void ObjectParameterGroupDesc::setGroupId(uint32_t ni){
  setId( ni );
}

void ObjectParameterGroupDesc::addParameter(const ObjectParameterDesc &op){
  parameters.push_back(op);
}

void ObjectParameterGroupDesc::addParameter(uint32_t type, const std::string& name, const std::string& desc){
  ObjectParameterDesc op;
  op.setType(type);
  op.setName(name);
  op.setDescription(desc);
  addParameter(op);
}

void ObjectParameterGroupDesc::packObjectDescFrame(Frame * f) const{

  f->packInt(id);
  f->packString(name);
  f->packString(desc);
  f->packInt(parameters.size());
  for(std::list<ObjectParameterDesc>::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    (*itcurr).packObjectDescFrame(f);
  }
}

ObjectParameterGroupPtr ObjectParameterGroupDesc::createObjectParameterGroup() const{
  ObjectParameterGroupPtr pg(new ObjectParameterGroupData());
  pg->setGroupId(id);
  for(std::list<ObjectParameterDesc>::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    ObjectParameter* param = NULL;
    switch((*itcurr).getType()){
      case obpT_Position_3D:
        param = new Position3dObjectParam();
        break;
      case obpT_Velocity:
        param = new Velocity3dObjectParam();
        break;
      case obpT_Order_Queue:
        param = new OrderQueueObjectParam();
        break;
      case obpT_Resource_List:
        param = new ResourceListObjectParam();
        break;
      case obpT_Reference:
        param = new ReferenceObjectParam();
        break;
      case obpT_Reference_Quantity_List:
        param = new RefQuantityListObjectParam();
        break;
      case obpT_Integer:
        param = new IntegerObjectParam();
        break;
      case obpT_Size:
        param = new SizeObjectParam();
        break;
    case obpT_Media:
        param = new MediaObjectParam();
        break;
        
      default:
        Logger::getLogger()->warning("Unknown ObjectParameter type %d in creating ParameterGroup", (*itcurr).getType());
        throw new std::exception();
        break;
    }
    pg->addParameter(param);
  }
  return pg;
}
