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

namespace RiskRuleset {

using std::string;
using std::pair;
using std::make_pair;

ResourceListParam::ResourceListParam(ObjectParameter* resList)
	: resources(dynamic_cast<ResourceListObjectParam*>(resList)) {
}

ResourceListParam::ResourceListParam(ResourceListObjectParam& resList)
	:resources(&resList) {
}

ResourceListParam::~ResourceListParam() {
}

ResourceListParam::operator ResourceListObjectParam&() { return *resources; }
ResourceListParam::operator const ResourceListObjectParam&() const  { return *resources; }

const bool ResourceListParam::hasResource(uint32_t resTypeId) const {
	std::map<uint32_t, std::pair<uint32_t, uint32_t> > res = resources->getResources();
	return res.find(resTypeId) != res.end();
}

const pair<uint32_t, uint32_t> ResourceListParam::getResource(uint32_t resTypeId) const {
   return hasResource(resTypeId)? resources->getResources().find(resTypeId)->second : make_pair(0u, 0u);
}

void ResourceListParam::setResource(uint32_t resTypeId, uint32_t currVal, uint32_t maxVal) {
	
	pair<uint32_t,uint32_t> res = getResource(resTypeId);

	if(currVal != KEEP_VAL)
   	res.first = currVal;
   
   if(maxVal != KEEP_VAL)
      res.second = maxVal;

	std::map<uint32_t, std::pair<uint32_t, uint32_t> > newRes = resources->getResources();
	newRes[resTypeId] = res;
	resources->setResources(newRes);
}

void ResourceListParam::addResource(uint32_t resTypeId, uint32_t val) {
	setResource( resTypeId, getResource(resTypeId).first + val );
}

// std::string helper versions

const uint32_t getTypeId(const std::string& resTypeName) {
	return Game::getGame()->getResourceManager()->
			getResourceDescription(resTypeName)->getResourceType();
}

const bool hasResource(const ResourceListParam& res, const std::string& resTypeName) {
	return res.hasResource( getTypeId(resTypeName) );
}

const std::pair<uint32_t,uint32_t> getResource(const ResourceListParam& resources,
															   const std::string& resTypeName) {
	return resources.getResource(getTypeId(resTypeName));
}
void setResource( ResourceListParam& resources,	const std::string& resTypeName,
						 uint32_t currVal, uint32_t maxVal) {
	resources.setResource(getTypeId(resTypeName), currVal, maxVal);
}

}
