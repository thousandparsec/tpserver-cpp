#ifndef OBJECT_H
#define OBJECT_H
/*  In Game Object class
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

#include <tpserver/common.h>
#include <tpserver/objectrelationships.h>
#include <tpserver/objectparametergroup.h>
#include <tpserver/protocolobject.h>

class ObjectBehaviour;

class IGObject : public ProtocolObject {

  public:
    IGObject( uint32_t newid );
    
    ~IGObject();
 
    // TODO: remove and replace with getId
    uint32_t getID() const;
    uint32_t getType() const;
    bool isAlive() const;
    uint32_t getTurn() const;
    

    uint32_t getParent() const;

    // mods time
    void setType(uint32_t newtype);
    // mods time
    void setName(const std::string &newname);
    // mods time
    void setDescription(const std::string &newdesc);
    void setIsAlive(bool ia);
    void setTurn(uint32_t nt);
    
    void removeFromParent();
    void addToParent(uint32_t pid);

    int getContainerType();
    IdSet getContainedObjects();
    void addContainedObject(uint32_t addObjectID);
    void removeContainedObject(uint32_t removeObjectID);

    ObjectParameter* getParameter(uint32_t groupnum, uint32_t paramnum) const;
    ObjectParameter* getParameterByType(uint32_t type) const;
    
    void setParameterGroup(const ObjectParameterGroupData::Ptr &ng);
    std::map<uint32_t, ObjectParameterGroupData::Ptr> getParameterGroups() const;
    
    ObjectBehaviour* getObjectBehaviour() const;
    void setObjectBehaviour(ObjectBehaviour* nob);
    
    virtual void setIsDirty(bool id);
    virtual bool isDirty() const;
    // Only Persistence classes should call these
    void setParent(uint32_t pid);

  private:
    uint32_t turn;
    bool alive;
    uint32_t type;
    
    ObjectRelationshipsData::Ptr relationships;
    std::map<uint32_t, ObjectParameterGroupData::Ptr> parameters;
    ObjectBehaviour* behaviour;

};

#endif
