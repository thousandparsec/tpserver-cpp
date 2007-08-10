#ifndef productioninfo_H
#define productioninfo_H
/*  production info
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

#include <map>
#include <string>

namespace RFTS_ {

class ProductionInfo
{
 public:   
   ProductionInfo();
   virtual ~ProductionInfo();
   
   virtual void init();

   const uint32_t getResourceCost(const std::string& resTypeName) const;
   const uint32_t getMaxResources(const std::string& resTypeName) const;
   const uint32_t getMinResources(const std::string& resTypeName) const;

 private:  

   std::map<std::string, uint32_t> resourceCost;
   std::map<std::string, std::pair<uint32_t,uint32_t> > minMaxResource;
};

}

#endif
