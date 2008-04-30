#ifndef resourcelistparam_H
#define resourcelistparam_H
/*  resourcelistparam class
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

#include <tpserver/resourcelistobjectparam.h>

namespace RFTS_ {

class ResourceListParam {

	ResourceListObjectParam* resources;
			
public:

	ResourceListParam(ObjectParameter* resList);
	ResourceListParam(ResourceListObjectParam& resList);
	~ResourceListParam();
	
	operator ResourceListObjectParam&();
	operator const ResourceListObjectParam&() const;

	const bool hasResource(uint32_t resTypeid) const;

	const std::pair<uint32_t,uint32_t> getResource(uint32_t resTypeId) const;

	void setResource(uint32_t resTypeId, uint32_t currVal, uint32_t maxVal = KEEP_VAL);
	
	void addResource(uint32_t resTypeId, uint32_t val);

	static const uint32_t KEEP_VAL = 0xFFFFFFFF;
};

// std::string convenience fns
const uint32_t getTypeId(const std::string& resTypeName);

const bool hasResource(const ResourceListParam& resources, const std::string& resTypeName);
const std::pair<uint32_t,uint32_t> getResource(const ResourceListParam& resources,
															   const std::string& resTypeName);
void setResource( ResourceListParam& resources, const std::string& resTypeName,
						uint32_t currVal, uint32_t maxVal = ResourceListParam::KEEP_VAL);

}

#endif
