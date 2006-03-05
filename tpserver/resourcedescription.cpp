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
{}


ResourceDescription::~ResourceDescription()
{}

void ResourceDescription::setResourceType(uint32_t resid){
    restype = resid;
}

void ResourceDescription::setNameSingular(const std::string& name){
    name_sig = name;
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

void ResourceDescription::setDescription(const std::string& desc){
    description = desc;
}

void ResourceDescription::setMass(uint32_t nm){
    mass = nm;
}

void ResourceDescription::setVolume(uint32_t nv){
    volume = nv;
}

void ResourceDescription::setModTime(uint64_t nmt){
    modtime = nmt;
}

uint32_t ResourceDescription::getResourceType() const{
    return restype;
}

std::string ResourceDescription::getNameSingular() const{
    return name_sig;
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

std::string ResourceDescription::getDescription() const{
    return description;
}

uint32_t ResourceDescription::getMass() const{
    return mass;
}

uint32_t ResourceDescription::getVolume() const{
    return volume;
}

uint64_t ResourceDescription::getModTime() const{
    return modtime;
}

