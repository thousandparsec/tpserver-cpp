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

#include <tpserver/game.h>
#include <tpserver/prng.h>

#include "productioninfo.h"

namespace RFTS_ {

using std::string;
using std::pair;

ProductionInfo::ProductionInfo() {
   init();
}

ProductionInfo::~ProductionInfo() {
}

void ProductionInfo::init() {
   
   // Resource Cost
   // stats
   resourceCost["Industry"] = 10;
   resourceCost["Social Environment"] = 4;
   resourceCost["Planetary Environment"] = 8;
   resourceCost["Population Maintenance"] = 1;
   resourceCost["Colonist"] = 5;

   // ships
   resourceCost["Scout"] = 3;
   resourceCost["Transport"] = 5;
   resourceCost["Mark1"] = 14;
   resourceCost["Mark2"] = 30;
   resourceCost["Mark3"] = 80;
   resourceCost["Mark4"] = 120;

   // pdb
   resourceCost["PDB1"] = 4;
   resourceCost["PDB2"] = 8;
   resourceCost["PDB3"] = 16;
   resourceCost["PDB1 Maintenance"] = 1;
   resourceCost["PDB2 Maintenance"] = 2;
   resourceCost["PDB3 Maintenance"] = 2;

   // ship tech
   resourceCost["Ship Technology"] = 1;


   // Min/Max Resources
   minMaxResource["Industry"] = pair<uint32_t,uint32_t>(40,70);
   minMaxResource["Population"] = pair<uint32_t, uint32_t>(30,110);
   minMaxResource["Social Environment"] = pair<uint32_t,uint32_t>(55,95);
   minMaxResource["Planetary Environment"] = pair<uint32_t,uint32_t>(35,70);
}

const uint32_t ProductionInfo::getResourceCost(const string& resTypeName) const {
   return (*resourceCost.find(resTypeName)).second;
}

const uint32_t ProductionInfo::getMinResources(const string& resTypeName) const {
   return (*minMaxResource.find(resTypeName)).second.first;
}

const uint32_t ProductionInfo::getMaxResources(const string& resTypeName) const {
   return (*minMaxResource.find(resTypeName)).second.second;
}

const uint32_t ProductionInfo::getRandResourceVal(const string& resTypeName) const {
  return Game::getGame()->getRandom()->getInRange(getMinResources(resTypeName),
                                                   getMaxResources(resTypeName));
}

}
