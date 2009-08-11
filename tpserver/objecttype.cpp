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
#include "objectbehaviour.h"
#include "algorithms.h"
#include <boost/bind.hpp>

#include "objecttype.h"

ObjectType::ObjectType( const std::string& nname, const std::string& ndesc ) : 
  ProtocolObject( ft04_ObjectDesc, 0, nname, ndesc ), nextparamgroupid(1)
{
  touchModTime();
}

ObjectType::~ObjectType(){
}

uint32_t ObjectType::getType() const{
  return id;
}

void ObjectType::setType(uint32_t nt){
  id = nt;
}

void ObjectType::pack(OutputFrame* frame) const
{
  ProtocolObject::pack( frame );
  frame->packInt64(getModTime());
  frame->packInt(paramgroups.size());
  for_each_value( paramgroups.begin(), paramgroups.end(), boost::bind( &ObjectParameterGroupDesc::pack, _1, frame ) );
}

void ObjectType::setupObject(IGObject::Ptr obj) const{
  for(std::map<uint32_t, ObjectParameterGroupDesc::Ptr>::const_iterator itcurr = paramgroups.begin(); itcurr != paramgroups.end();
      ++itcurr){
    obj->setParameterGroup((itcurr->second)->createObjectParameterGroup());
  }
  ObjectBehaviour* ob = createObjectBehaviour();
  // TODO: behaviour should have weak_ptr!
  ob->setObject(obj.get());
  obj->setObjectBehaviour(ob);
  ob->setupObject();
}

ObjectParameterGroupDesc::Ptr ObjectType::createParameterGroupDesc( const std::string& gname, const std::string& gdesc )
{
  ObjectParameterGroupDesc::Ptr result( new ObjectParameterGroupDesc( nextparamgroupid, gname, gdesc ) );
  paramgroups[nextparamgroupid] = result;
  nextparamgroupid++;
  touchModTime();
  return result;
}

ObjectParameterGroupDesc::Ptr ObjectType::getParameterGroupDesc(uint32_t groupid) const{
  return find_default( paramgroups, groupid, ObjectParameterGroupDesc::Ptr() );
}

