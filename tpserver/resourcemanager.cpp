/*  Resource Manager
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

#include "game.h"
#include "persistence.h"

#include "resourcemanager.h"
#include "algorithms.h"

ResourceManager::ResourceManager(){
    nextid = 1;
}


ResourceManager::~ResourceManager()
{}

void ResourceManager::init(){
    Persistence* persist = Game::getGame()->getPersistence();
    nextid = persist->getMaxResourceId() + 1;
    IdSet ridset(persist->getResourceIds());
    fill_by_set( resdescs, ridset, ResourceDescription::Ptr() );
}

uint32_t ResourceManager::addResourceDescription(ResourceDescription::Ptr res){
    res->setResourceType(nextid++);
    res->touchModTime();
    resdescs[res->getResourceType()] = res;
    Game::getGame()->getPersistence()->saveResource(res);
    return res->getResourceType();
}

uint32_t ResourceManager::addResourceDescription( const std::string& nname, const std::string& nunit, const std::string& ndesc) {
    return addResourceDescription( ResourceDescription::Ptr( new ResourceDescription( nname, nunit, ndesc ) ) );
}


const ResourceDescription::Ptr ResourceManager::getResourceDescription(uint32_t restype){
  ResourceDescription::Ptr rtn = find_default( resdescs, restype, ResourceDescription::Ptr() );
    if ( !rtn ) {
        rtn = Game::getGame()->getPersistence()->retrieveResource(restype);
        resdescs[restype] = rtn;
    }
    return rtn;
}

const ResourceDescription::Ptr ResourceManager::getResourceDescription(const std::string& restype){
  for(ResourceMap::iterator rl = resdescs.begin();
      rl != resdescs.end(); ++rl){
    if(rl->second){
      if(rl->second->getNameSingular() == restype){
        return rl->second;
      }
    }
  }
  return ResourceDescription::Ptr();
}

IdSet ResourceManager::getAllIds(){
    return generate_key_set( resdescs );
}

