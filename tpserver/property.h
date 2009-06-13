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

#include <string>
#include <set>

class Frame;

class Property {
  public:
    /**
     * Default constructor
     *
     * Sets timestamp
     */
    Property();

    /**
     * Pack property data into a frame
     */
    void packFrame(Frame* frame) const;

    uint32_t getPropertyId() const;
    std::set<uint32_t> getCategoryIds() const;
    bool isInCategory(uint32_t catid) const;
    uint32_t getRank() const;
    std::string getName() const;
    std::string getDisplayName() const;
    std::string getDescription() const;
    std::string getTpclDisplayFunction() const;
    std::string getTpclRequirementsFunction() const;
    uint64_t getModTime() const;

    void setPropertyId(uint32_t id);
    void addCategoryId(uint32_t id);
    void setCategoryIds(const std::set<uint32_t>& idset);
    void setRank(uint32_t r);
    void setName(const std::string& n);
    void setDisplayName(const std::string& d);
    void setDescription(const std::string& d);
    void setTpclDisplayFunction(const std::string& d);
    void setTpclRequirementsFunction(const std::string& r);
    void setModTime(uint64_t nmt);

  private:
    uint32_t propid;
    std::set<uint32_t> catids;
    uint32_t rank;
    uint64_t timestamp;
    std::string name;
    std::string display;
    std::string description;
    std::string tpcl_display;
    std::string tpcl_requires;

};

#endif
