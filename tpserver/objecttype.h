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

#include <stdint.h>
#include <map>
#include <string>
#include <tpserver/modifiable.h>

class Frame;
class IGObject;
class ObjectParameterGroupDesc;
class ObjectBehaviour;

class ObjectType : public Modifiable {

  public:
    ObjectType();
    virtual ~ObjectType();

    uint32_t getType() const;
    std::string getTypeName() const;

    void setType(uint32_t nt);
    void packObjectDescFrame(Frame* frame);
    void setupObject(IGObject* obj) const;

  protected:
    void addParameterGroupDesc(ObjectParameterGroupDesc* group);
    ObjectParameterGroupDesc* getParameterGroupDesc(uint32_t groupid) const;
    virtual ObjectBehaviour* createObjectBehaviour() const = 0;

    std::string nametype;
    std::string typedesc;

  private:
    uint32_t type;
    uint32_t nextparamgroupid;
    std::map<uint32_t, ObjectParameterGroupDesc*> paramgroups;
};

#endif
