
#include <math.h>

#include "frame.h"

#include "vector3d.h"

Vector3d::Vector3d(){

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

long long Vector3d::getDistance(const Vector3d & origin) const{
  return (long long)sqrt(getDistanceSq(origin));
}

long long Vector3d::getDistanceSq(const Vector3d & origin) const{
  long long dx = x - origin.x;
  long long dy = y - origin.y;
  long long dz = z - origin.z;
  return (unsigned long long)(dx * dx) + (unsigned long long)(dy * dy) + (unsigned long long)(dz * dz);
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
