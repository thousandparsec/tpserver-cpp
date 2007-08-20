/*  map
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

#include <cmath>
#include <utility>

#include <tpserver/game.h>
#include <tpserver/prng.h>

#include "staticobject.h"

#include "map.h"

namespace RFTS_ {

using std::pair;

// used for calculating universe (for wrapping map)
namespace {
   const uint64_t UNIVERSE_TOTAL_SCALE = 1050000000ll;
   const uint32_t UNIVERSE_WIDTH = 4;
   const uint32_t UNIVERSE_HEIGHT = 3;
}


const Vector3d getUniverseCoord(double unitX, double unitY) {
   return Vector3d( static_cast<uint64_t>(UNIVERSE_WIDTH * UNIVERSE_TOTAL_SCALE * unitX),
                    static_cast<uint64_t>(UNIVERSE_HEIGHT * UNIVERSE_TOTAL_SCALE * unitY),
                    0 );
}

const Vector3d getUniverseCoord(const pair<double,double>& unitPos) {
   return Vector3d( static_cast<uint64_t>(UNIVERSE_WIDTH * UNIVERSE_TOTAL_SCALE * unitPos.first),
                    static_cast<uint64_t>(UNIVERSE_HEIGHT * UNIVERSE_TOTAL_SCALE * unitPos.second),
                    0 );
}

const Vector3d getRandPlanetOffset() {
   Random *rand = Game::getGame()->getRandom();
   return Vector3d( static_cast<int64_t>( UNIVERSE_WIDTH * UNIVERSE_TOTAL_SCALE *
                        (rand->getInRange(-100,100)/10000.) ),
                    static_cast<int64_t>( UNIVERSE_HEIGHT * UNIVERSE_TOTAL_SCALE *
                        (rand->getInRange(-100,100)/10000.) ),
                    0 );
}

const double getWrappingUnitDist(const StaticObject& obj1, const StaticObject& obj2) {

   pair<double,double> pos1 = obj1.getUnitPos();
   pair<double,double> pos2 = obj2.getUnitPos();

   double x = std::abs(pos1.first - pos2.first);
   if(x > .5)
      x = 1 - x;

   double y = std::abs(pos1.second - pos2.second);
   if(y > .5)
      y = 1 - y;

   return std::sqrt(x*x + y*y);
}

const unsigned getWrappingDistSq(const StaticObject& obj1, const StaticObject& obj2) {

   pair<double,double> pos1 = obj1.getUnitPos();
   pair<double,double> pos2 = obj2.getUnitPos();

   double x = std::abs(pos1.first - pos2.first);
   if(x > .5)
      x = 1 - x;

   double y = std::abs(pos1.second - pos2.second);
   if(y > .5)
      y = 1 - y;

   x *= UNIVERSE_WIDTH * UNIVERSE_TOTAL_SCALE;
   y *= UNIVERSE_HEIGHT * UNIVERSE_TOTAL_SCALE;

   return static_cast<unsigned>(x*x + y*y);
}


}
