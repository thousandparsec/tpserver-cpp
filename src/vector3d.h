#ifndef VECTOR3D_H
#define VECTOR3D_H

class Frame;

class Vector3d{

 public:
  Vector3d();
  Vector3d(const Vector3d & rhs);
  Vector3d(long long x, long long y, long long z);

  Vector3d operator=(const Vector3d & rhs);
  Vector3d operator+(const Vector3d & rhs) const;
  Vector3d operator-(const Vector3d & rhs) const;
  Vector3d operator*(long long val) const;
  bool operator==(const Vector3d &rhs) const;
  bool operator!=(const Vector3d &rhs) const;
  Vector3d makeLength(long long length) const;

  long long getX() const;
  long long getY() const;
  long long getZ() const;

  void setAll(long long newx, long long newy, long long newz);

  unsigned long long getDistance(const Vector3d & origin) const;
  double getDistanceSq(const Vector3d & origin) const;

  void pack(Frame * frame) const;
  void unpack(Frame * frame);

 private:
  long long x;
  long long y;
  long long z;
  


};


#endif
