#ifndef DESIGNVIEW_H
#define DESIGNVIEW_H
/*  DesignView class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

class Frame;

class DesignView : public Modifiable {
  public:
    DesignView();
    DesignView( uint32_t desid, bool visibility );
    virtual ~DesignView();

    void packFrame(Frame* frame) const;

    uint32_t getDesignId() const;
    bool isCompletelyVisible() const;

    std::string getVisibleName() const;
    bool canSeeName() const;
    std::string getVisibleDescription() const;
    bool canSeeDescription() const;
    uint32_t getVisibleOwner() const;
    bool canSeeOwner() const;
    std::map<uint32_t, uint32_t> getVisibleComponents() const;
    uint32_t getVisibleNumExist() const;
    bool canSeeNumExist() const;
    PropertyValue::Map getVisiblePropertyValues() const;


    void setDesignId(uint32_t id);
    void setIsCompletelyVisible(bool ncv);

    void setVisibleName(const std::string& n);
    void setCanSeeName(bool csn);
    void setVisibleDescription(const std::string& d);
    void setCanSeeDescription(bool csd);
    void setVisibleOwner(uint32_t o);
    void setCanSeeOwner(bool cso);
    void setVisibleComponents(std::map<uint32_t, uint32_t> cl);
    void setVisibleNumExist(uint32_t nne);
    void setCanSeeNumExist(bool csn);
    void setVisiblePropertyValues(PropertyValue::Map pvl);

  protected:

    uint32_t designid;

    bool completelyvisible;

    bool seename;
    std::string name;
    bool seedesc;
    std::string description;

    bool seenum;
    uint32_t exist;
    bool seeowner;
    uint32_t owner;

    std::map<uint32_t, uint32_t> components;
    PropertyValue::Map properties;

};

#endif
