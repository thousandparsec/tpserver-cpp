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
#include "objectparametergroup.h"

#include "objectparametergroupdesc.h"
#include <boost/bind.hpp>

ObjectParameterGroupDesc::ObjectParameterGroupDesc( uint32_t nid, const std::string& pname, const std::string& pdesc ) 
  : Describable(nid, pname, pdesc) {
}

void ObjectParameterGroupDesc::addParameter(uint32_t ptype, const std::string& pname, const std::string& pdesc){
  ParameterDesc op( ptype, pname, pdesc );
  parameters.push_back(op);
}

void ObjectParameterGroupDesc::pack(OutputFrame::Ptr f) const{

  f->packInt(id);
  f->packString(name);
  f->packString(desc);
  f->packInt(parameters.size());
  for ( Parameters::const_iterator it = parameters.begin(); it != parameters.end(); ++it )
  {
    uint32_t pid = it->get<0>();
    f->packString(it->get<1>());
    f->packInt(pid);
    f->packString(it->get<2>());
  }
}

ObjectParameterGroup::Ptr ObjectParameterGroupDesc::createObjectParameterGroup() const{
  ObjectParameterGroup::Ptr pg(new ObjectParameterGroup());
  pg->setGroupId(id);
  for(Parameters::const_iterator itcurr = parameters.begin();
      itcurr != parameters.end(); ++itcurr){
    ObjectParameter* param = NULL;
    switch(itcurr->get<0>()){
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
        Logger::getLogger()->warning("Unknown ObjectParameter type %d in creating ParameterGroup", itcurr->get<0>());
        return ObjectParameterGroup::Ptr();
    }
    pg->addParameter(param);
  }
  return pg;
}
