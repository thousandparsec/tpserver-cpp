/*  3D vector with long long components
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

#include "frame.h"

#include "vector3d.h"

Vector3d::Vector3d(){
  x = y = z = 0LL;
}

Vector3d::Vector3d(long long newx, long long newy, long long newz){
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

Vector3d Vector3d::operator*(long long val) const{
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

Vector3d Vector3d::makeLength(long long length) const{
  Vector3d rtn;
  double len = sqrt(((double)x * (double)x) + ((double)y * (double)y) + ((double)z * (double)z));
  rtn.x = (long long)((double)x * (double)length / len);
  rtn.y = (long long)((double)y * (double)length / len);
  rtn.z = (long long)((double)z * (double)length / len);
  return rtn;
}

long long Vector3d::getX() const{
  return x;
}

long long Vector3d::getY() const{
  return y;
}

long long Vector3d::getZ() const{
  return z;
}

void Vector3d::setAll(long long newx, long long newy, long long newz){
  x = newx;
  y = newy;
  z = newz;
}

unsigned long long Vector3d::getDistance(const Vector3d & origin) const{
  return (unsigned long long)sqrt(getDistanceSq(origin));
}

double Vector3d::getDistanceSq(const Vector3d & origin) const{
  double dx = (double)x - (double)origin.x;
  double dy = (double)y - (double)origin.y;
  double dz = (double)z - (double)origin.z;
  return ((dx * dx) + (dy * dy) + (dz * dz));
}

void Vector3d::pack(Frame * frame) const{
  frame->packInt64(x);
  frame->packInt64(y);
  frame->packInt64(z);
}

void Vector3d::unpack(Frame * frame){
  x = frame->unpackInt64();
  y = frame->unpackInt64();
  z = frame->unpackInt64();
}
