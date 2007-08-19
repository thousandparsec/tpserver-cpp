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
   minMaxResource[pair<string,PlanetType>("Population",INDUSTRIAL)] = pair<uint32_t,uint32_t>(10,30);
   minMaxResource[pair<string,PlanetType>("Industry",INDUSTRIAL)] = pair<uint32_t,uint32_t>(25,90);
   minMaxResource[pair<string,PlanetType>("Social Environment",INDUSTRIAL)] = pair<uint32_t,uint32_t>(5,20);
   minMaxResource[pair<string,PlanetType>("Planetary Environment",INDUSTRIAL)] = pair<uint32_t,uint32_t>(5,20);

   minMaxResource[pair<string,PlanetType>("Population",NEUTRAL)] = pair<uint32_t,uint32_t>(40,80);
   minMaxResource[pair<string,PlanetType>("Industry",NEUTRAL)] = pair<uint32_t,uint32_t>(30,70);
   minMaxResource[pair<string,PlanetType>("Social Environment",NEUTRAL)] = pair<uint32_t,uint32_t>(30,75);
   minMaxResource[pair<string,PlanetType>("Planetary Environment",NEUTRAL)] = pair<uint32_t,uint32_t>(30,75);

   minMaxResource[pair<string,PlanetType>("Population",PRIMARY)] = pair<uint32_t,uint32_t>(75,100);
   minMaxResource[pair<string,PlanetType>("Industry",PRIMARY)] = pair<uint32_t,uint32_t>(30,55);
   minMaxResource[pair<string,PlanetType>("Social Environment",PRIMARY)] = pair<uint32_t,uint32_t>(55,100);
   minMaxResource[pair<string,PlanetType>("Planetary Environment",PRIMARY)] = pair<uint32_t,uint32_t>(55,100);
}

const uint32_t ProductionInfo::getResourceCost(const string& resTypeName) const {
   return (*resourceCost.find(resTypeName)).second;
}

const uint32_t ProductionInfo::getMinResources(const string& resTypeName, PlanetType planetType) const {
   return (*minMaxResource.find( pair<string,PlanetType>(resTypeName,planetType)) ).second.first;
}

const uint32_t ProductionInfo::getMaxResources(const string& resTypeName, PlanetType planetType) const {
   return (*minMaxResource.find( pair<string,PlanetType>(resTypeName,planetType)) ).second.second;
}

const uint32_t ProductionInfo::getRandResourceVal(const string& resTypeName, PlanetType planetType) const {
   uint32_t val = Game::getGame()->getRandom()->getInRange( getMinResources(resTypeName,planetType),
                                                   getMaxResources(resTypeName,planetType));
  return val - (val % 5); // round to a mult of 5
}

}
