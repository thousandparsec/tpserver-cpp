#ifndef PROPERTY_H
#define PROPERTY_H
/*  Component/Design Property class
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

#include <tpserver/protocolobject.h>
#include <tpserver/common.h>

class Property : public ProtocolObject {
  public:
    typedef boost::shared_ptr<Property> Ptr;
    /**
     * Default constructor
     */
    // TODO: should take proper parameters
    Property();


    /**
     * Pack property data into a frame
     */
    void pack(OutputFrame::Ptr frame) const;

    /// TODO: remove
    uint32_t getPropertyId() const;
    IdSet getCategoryIds() const;
    bool isInCategory(uint32_t catid) const;
    uint32_t getRank() const;
    std::string getDisplayName() const;
    std::string getTpclDisplayFunction() const;
    std::string getTpclRequirementsFunction() const;

    /// TODO: remove
    void setPropertyId(uint32_t id);
    void addCategoryId(uint32_t id);
    void setCategoryIds(const std::set<uint32_t>& idset);
    void setRank(uint32_t r);
    void setDisplayName(const std::string& d);
    void setTpclDisplayFunction(const std::string& d);
    void setTpclRequirementsFunction(const std::string& r);

  private:
    IdSet catids;
    uint32_t rank;
    std::string display;
    std::string tpcl_display;
    std::string tpcl_requires;

};

#endif
