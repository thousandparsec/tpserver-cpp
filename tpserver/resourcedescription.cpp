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

#include "resourcedescription.h"

ResourceDescription::ResourceDescription()
  : ProtocolObject(ft02_ResDesc,0), mass( 0 ), volume( 0 )
{}

ResourceDescription::ResourceDescription( const std::string& nname, const std::string& nunit, const std::string& ndesc )
  : ProtocolObject(ft02_ResDesc,0,nname,ndesc), mass( 0 ), volume( 0 )
{
  name_plur = nname + "s";
  unit_sig = nunit;
  unit_plur = nunit + "s";
}


ResourceDescription::~ResourceDescription()
{}

void ResourceDescription::setResourceType(uint32_t resid){
    setId( resid );
}

void ResourceDescription::setNameSingular(const std::string& name){
    setName( name );
}

void ResourceDescription::setNamePlural(const std::string& name){
    name_plur = name;
}

void ResourceDescription::setUnitSingular(const std::string& unit){
    unit_sig = unit;
}

void ResourceDescription::setUnitPlural(const std::string& unit){
    unit_plur = unit;
}

void ResourceDescription::setMass(uint32_t nm){
    mass = nm;
}

void ResourceDescription::setVolume(uint32_t nv){
    volume = nv;
}

uint32_t ResourceDescription::getResourceType() const{
    return id;
}

std::string ResourceDescription::getNameSingular() const{
    return name;
}

std::string ResourceDescription::getNamePlural() const{
    return name_plur;
}

std::string ResourceDescription::getUnitSingular() const{
    return unit_sig;
}
    
std::string ResourceDescription::getUnitPlural() const{
    return unit_plur;
}

uint32_t ResourceDescription::getMass() const{
    return mass;
}

uint32_t ResourceDescription::getVolume() const{
    return volume;
}

void ResourceDescription::pack(OutputFrame::Ptr frame) const{
    frame->setType(ft02_ResDesc);
    frame->packInt(id);
    frame->packString(name);
    frame->packString(name_plur);
    frame->packString(unit_sig);
    frame->packString(unit_plur);
    frame->packString(desc);
    frame->packInt(mass);
    frame->packInt(volume);
    frame->packInt64(getModTime());
}
