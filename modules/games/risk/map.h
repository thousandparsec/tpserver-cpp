#ifndef map_H
#define map_H
/*  map class
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


#include <tpserver/vector3d.h>

namespace RiskRuleset {

class StaticObject;


const Vector3d getUniverseCoord(double unitX, double unitY);
const Vector3d getUniverseCoord(const std::pair<double,double>& unitPos);

const Vector3d getRandPlanetOffset();

const double getWrappingUnitDist(const StaticObject& obj1, const StaticObject& obj2);
const unsigned getWrappingDistSq(const StaticObject& obj1, const StaticObject& obj2);

}

#endif
