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

#include <stdint.h>

#include "resourcedescription.h"
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
    fill_by_set( resdescs, ridset, NULL );
}

uint32_t ResourceManager::addResourceDescription(ResourceDescription* res){
    res->setResourceType(nextid++);
    res->touchModTime();
    resdescs[res->getResourceType()] = res;
    Game::getGame()->getPersistence()->saveResource(res);
    return res->getResourceType();
}

const ResourceDescription* ResourceManager::getResourceDescription(uint32_t restype){
    ResourceDescription* rtn = find_default( resdescs, restype, NULL );
    if ( rtn == NULL ) {
        rtn = Game::getGame()->getPersistence()->retrieveResource(restype);
        resdescs[restype] = rtn;
    }
    return rtn;
}

const ResourceDescription* ResourceManager::getResourceDescription(const std::string& restype){
  for(ResourceMap::iterator rl = resdescs.begin();
      rl != resdescs.end(); ++rl){
    if(rl->second != NULL){
      if(rl->second->getNameSingular() == restype){
        return rl->second;
      }
    }
  }
  return NULL;
}

IdSet ResourceManager::getAllIds(){
    return generate_key_set( resdescs );
}

