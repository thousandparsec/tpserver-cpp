#ifndef DESIGN_H
#define DESIGN_H
/*  Design class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <string>
#include <map>
#include <stdint.h>

#include <tpserver/propertyvalue.h>
#include <tpserver/modifiable.h>
#include <tpserver/describable.h>
#include <tpserver/common.h>

class Design : public Modifiable, public Describable {
  public:
    Design();
    virtual ~Design();

    // TODO: remove
    uint32_t getDesignId() const;
    uint32_t getCategoryId() const;
    uint32_t getOwner() const;
    IdMap getComponents() const;
    uint32_t getNumExist() const;
    uint32_t getInUse() const;
    bool isValid() const;
    std::string getFeedback() const;
    double getPropertyValue(uint32_t propid) const;
    PropertyValue::Map getPropertyValues() const;

    // TODO: remove
    void setDesignId(uint32_t id);
    void setCategoryId(uint32_t id);
    void setOwner(uint32_t o);
    void setComponents(IdMap cl);
    void setInUse(uint32_t niu);
    void setNumExist(uint32_t nne);

    void eval();
    void setPropertyValues(PropertyValue::Map pvl);
    void setValid(bool v, const std::string& f);

    void addUnderConstruction(uint32_t num);
    void addComplete(uint32_t num);
    void removeCanceledConstruction(uint32_t num);
    void removeDestroyed(uint32_t num);

  protected:
    uint32_t catid;
    uint32_t inuse;
    uint32_t exist;
    uint32_t owner;
    bool valid;
    IdMap components;
    PropertyValue::Map properties;
    std::string feedback;

};

#endif
