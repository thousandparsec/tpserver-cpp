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
#include <tpserver/protocolview.h>

class Frame;

class DesignView : public ProtocolView {
  public:
    typedef boost::shared_ptr< DesignView > Ptr;
    DesignView();
    DesignView( uint32_t desid, bool visibility );
    virtual ~DesignView();

    void pack(OutputFrame* frame) const;

    uint32_t getDesignId() const;
    
    uint32_t getVisibleOwner() const;
    bool canSeeOwner() const;
    IdMap getVisibleComponents() const;
    uint32_t getVisibleNumExist() const;
    bool canSeeNumExist() const;
    PropertyValue::Map getVisiblePropertyValues() const;

    void setDesignId(uint32_t id);

    void setVisibleOwner(uint32_t o);
    void setCanSeeOwner(bool cso);
    void setVisibleComponents(IdMap cl);
    void setVisibleNumExist(uint32_t nne);
    void setCanSeeNumExist(bool csn);
    void setVisiblePropertyValues(PropertyValue::Map pvl);

  protected:

    bool seenum;
    uint32_t exist;
    bool seeowner;
    uint32_t owner;

    IdMap components;
    PropertyValue::Map properties;

};

#endif
