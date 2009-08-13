#ifndef OBJECTTYPE_H
#define OBJECTTYPE_H
/*  ObjectType base class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/modifiable.h>
#include <tpserver/object.h>
#include <tpserver/protocolobject.h>
#include <tpserver/objectparametergroupdesc.h>

class ObjectBehaviour;

class ObjectType : public ProtocolObject {
  public:
    typedef boost::shared_ptr<ObjectType> Ptr;
    ObjectType( const std::string& nname, const std::string& ndesc );
    virtual ~ObjectType();

    uint32_t getType() const;

    void setType(uint32_t nt);
    void pack(OutputFrame::Ptr frame) const;
    void setupObject(IGObject::Ptr obj) const;

  protected:
    ObjectParameterGroupDesc::Ptr createParameterGroupDesc( const std::string& gname, const std::string& gdesc );
    ObjectParameterGroupDesc::Ptr getParameterGroupDesc(uint32_t groupid) const;
    virtual ObjectBehaviour* createObjectBehaviour() const = 0;

  private:
    uint32_t nextparamgroupid;
    std::map<uint32_t, ObjectParameterGroupDesc::Ptr> paramgroups;
};

#endif
