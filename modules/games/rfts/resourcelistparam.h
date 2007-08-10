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

class ResourceListParam : public ::ResourceListObjectParam
{
 public:
   ResourceListParam();
   virtual ~ResourceListParam();

   std::pair<uint32_t,uint32_t>& getResource(uint32_t resTypeId);
   std::pair<uint32_t,uint32_t>& getResource(const std::string& resTypeName);
   
   void setResource(uint32_t resTypeId, uint32_t currVal = 0, uint32_t maxVal = 0);
   void setResource(const std::string& resTypeName, uint32_t currVal = 0, uint32_t maxVal = 0);
   
};


}

#endif
