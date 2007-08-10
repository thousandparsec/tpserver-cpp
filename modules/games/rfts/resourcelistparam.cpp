/*  resourcelistparam
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include <cassert>

#include <tpserver/game.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>

#include "resourcelistparam.h"

namespace RFTS_ {

using std::string;
using std::pair;

ResourceListParam::ResourceListParam() : ResourceListObjectParam() {

}

ResourceListParam::~ResourceListParam() {

}

pair<uint32_t, uint32_t>& ResourceListParam::getResource(uint32_t resTypeId) {
   return resources[ resTypeId ];
}

pair<uint32_t, uint32_t>& ResourceListParam::getResource(const string& resTypeName) {
   return getResource( Game::getGame()->getResourceManager()->
                       getResourceDescription(resTypeName)->getResourceType() );
}

void ResourceListParam::setResource(uint32_t resTypeId, uint32_t currVal, uint32_t maxVal) {
   pair<uint32_t,uint32_t> &res = getResource(resTypeId);
   res.first = currVal;
   res.second = maxVal;
}

void ResourceListParam::setResource(const string& resTypeName, uint32_t currVal, uint32_t maxVal) {
   pair<uint32_t,uint32_t> &res = getResource(resTypeName);
   res.first = currVal;
   res.second = maxVal;
}

}
