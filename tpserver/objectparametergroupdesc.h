#ifndef OBJECTPARAMETERGROUPDESC_H
#define OBJCETPARAMETERGROUPDESC_H
/*  ObjectParameterGroupDesc class
 * Collects Object Parameters together into a group.
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <string>
#include <list>
#include <tpserver/objectparametergroup.h>
#include <tpserver/describable.h>

class Frame;
class ObjectParameter;

class ObjectParameterDesc{
  public:
    ObjectParameterDesc();
    ObjectParameterDesc(const ObjectParameterDesc& rhs);
    ~ObjectParameterDesc();
    
    ObjectParameterDesc& operator=(const ObjectParameterDesc& rhs);
    
    void setType(uint32_t nt);
    void setName(const std::string& nn);
    void setDescription(const std::string& nd);
    
    uint32_t getType() const;
    
    void packObjectDescFrame(Frame* f) const;
    
  private:
    uint32_t type;
    std::string name;
    std::string description;
  
};

class ObjectParameterGroupDesc : public Describable {
  public:
    ObjectParameterGroupDesc();
    ~ObjectParameterGroupDesc();

    //TODO: remove
    uint32_t getGroupId() const;

    // TODO: remove
    void setGroupId(uint32_t ni);

    void addParameter(const ObjectParameterDesc& op);
    void addParameter(uint32_t type, const std::string& name, const std::string& desc);

    void packObjectDescFrame(Frame* f) const;
    ObjectParameterGroupPtr createObjectParameterGroup() const;


  protected:
    std::list<ObjectParameterDesc> parameters;

};


#endif
