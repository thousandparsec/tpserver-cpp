/*  3D vector with int64_t components
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

#include <math.h>
#include <cassert>

#include "vector3d.h"

Vector3d::Vector3d(){
  x = y = z = 0LL;
}

Vector3d::Vector3d(int64_t newx, int64_t newy, int64_t newz){
  x = newx;
  y = newy;
  z = newz;
}

Vector3d::Vector3d(const Vector3d & rhs){
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;
}

Vector3d Vector3d::operator=(const Vector3d & rhs){
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;

  return *this;
}

Vector3d Vector3d::operator+(const Vector3d & rhs) const{
  Vector3d rtn;
  rtn.x = x + rhs.x;
  rtn.y = y + rhs.y;
  rtn.z = z + rhs.z;
  return rtn;
}

Vector3d Vector3d::operator-(const Vector3d & rhs) const{
  Vector3d rtn;
  rtn.x = x - rhs.x;
  rtn.y = y - rhs.y;
  rtn.z = z - rhs.z;
  return rtn;
}

Vector3d Vector3d::operator*(int64_t val) const{
  Vector3d rtn;
  rtn.x = x * val;
  rtn.y = y * val;
  rtn.z = z *val;
  return rtn;
}

bool Vector3d::operator==(const Vector3d &rhs) const{
  return (x == rhs.x && y == rhs.y && z == rhs.z);
}

bool Vector3d::operator!=(const Vector3d &rhs) const{
  return !(operator==(rhs));
}

uint64_t Vector3d::getLength() const {
  return (uint64_t) sqrt( getLengthSq() );
}

double Vector3d::getLengthSq() const {
  return ((double)x * (double)x) + ((double)y * (double)y) + ((double)z * (double)z);
}

Vector3d Vector3d::makeLength(int64_t length) const{
  double thisLength = sqrt(getLengthSq());
  assert(thisLength > 0);
  double scale = length / thisLength;
  return *this * scale;
}

int64_t Vector3d::getX() const{
  return x;
}

int64_t Vector3d::getY() const{
  return y;
}

int64_t Vector3d::getZ() const{
  return z;
}

void Vector3d::setAll(int64_t newx, int64_t newy, int64_t newz){
  x = newx;
  y = newy;
  z = newz;
}

uint64_t Vector3d::getDistance(const Vector3d & origin) const{
  return (uint64_t)sqrt(getDistanceSq(origin));
}

double Vector3d::getDistanceSq(const Vector3d & origin) const{
  Vector3d diff( *this - origin );
  return diff.getLengthSq();
}

void Vector3d::pack(OutputFrame::Ptr frame) const{
  frame->packInt64(x);
  frame->packInt64(y);
  frame->packInt64(z);
}

void Vector3d::unpack(InputFrame * frame){
  x = frame->unpackInt64();
  y = frame->unpackInt64();
  z = frame->unpackInt64();
}
