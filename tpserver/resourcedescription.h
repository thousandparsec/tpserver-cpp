/*  Resource Description 
 *
 *  Copyright (C) 2006  Lee Begg and the Thousand Parsec Project
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
#ifndef RESOURCEDESCRIPTION_H
#define RESOURCEDESCRIPTION_H

#include <string>
#include <tpserver/modifiable.h>

class Frame;

class ResourceDescription : public Modifiable
{
public:
    ResourceDescription();
    ~ResourceDescription();

    void setResourceType(uint32_t resid);
    void setNameSingular(const std::string& name);
    void setNamePlural(const std::string& name);
    void setUnitSingular(const std::string& unit);
    void setUnitPlural(const std::string& unit);
    void setDescription(const std::string& desc);
    void setMass(uint32_t nm);
    void setVolume(uint32_t nv);
    
    uint32_t getResourceType() const;
    std::string getNameSingular() const;
    std::string getNamePlural() const;
    std::string getUnitSingular() const;
    std::string getUnitPlural() const;
    std::string getDescription() const;
    uint32_t getMass() const;
    uint32_t getVolume() const;
    
    void packFrame(Frame* frame) const;

private:
    uint32_t restype;
    std::string name_sig, name_plur;
    std::string unit_sig, unit_plur;
    std::string description;
    uint32_t mass, volume;
};

#endif
