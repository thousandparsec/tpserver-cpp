#ifndef VECTOR3D_H
#define VECTOR3D_H
/*  Vector3d class
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

#include <stdint.h>

class Frame;

class Vector3d{

 public:
  Vector3d();
  Vector3d(const Vector3d & rhs);
  Vector3d(int64_t x, int64_t y, int64_t z);

  Vector3d operator=(const Vector3d & rhs);
  Vector3d operator+(const Vector3d & rhs) const;
  Vector3d operator-(const Vector3d & rhs) const;
  Vector3d operator*(int64_t val) const;
  bool operator==(const Vector3d &rhs) const;
  bool operator!=(const Vector3d &rhs) const;
  Vector3d makeLength(int64_t length) const;

  int64_t getX() const;
  int64_t getY() const;
  int64_t getZ() const;

  void setAll(int64_t newx, int64_t newy, int64_t newz);

  uint64_t getDistance(const Vector3d & origin) const;
  double getDistanceSq(const Vector3d & origin) const;

  void pack(Frame * frame) const;
  void unpack(Frame * frame);

 private:
  int64_t x;
  int64_t y;
  int64_t z;
  


};


#endif
