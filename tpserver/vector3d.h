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

/**
 * 3D Vector implementation based on 64-bit integers
 */
class Vector3d {
  public:
    /**
     * Default constructor
     *
     * Sets all values to zero.
     */
    Vector3d();

    /**
     * Copy constructor
     */
    Vector3d(const Vector3d & rhs);

    /**
     * Explicit value constructor
     */
    Vector3d(int64_t x, int64_t y, int64_t z);

    /**
     * Assignment operator
     */
    Vector3d operator=(const Vector3d & rhs);

    /**
     * Addition operator
     */
    Vector3d operator+(const Vector3d & rhs) const;

    /**
     * Subtraction operator
     */
    Vector3d operator-(const Vector3d & rhs) const;

    /**
     * Scalar multiplication operator
     */
    Vector3d operator*(int64_t val) const;

    /**
     * Equality operator
     */
    bool operator==(const Vector3d &rhs) const;

    /**
     * In-equality operator
     */
    bool operator!=(const Vector3d &rhs) const;

    /**
     * Returns the length of the vector
     *
     * @warning The result is truncated to a integral value.
     */
    uint64_t getLength() const;

    /**
     * Returns the length of the vector, squared
     */
    double getLengthSq() const;

    /**
     * Passed length vector creation
     *
     * Creates a vector with the same direction but length
     * as passed.
     *
     * @warning Function will assert fail if vector is of length zero.
     *
     * @param length the desired length of the vector
     * @returns Vector with the same direction as this but with the passed 
     *          length
     */
    Vector3d makeLength(int64_t length) const;

    /**
     * Returns the X component of the vector
     */
    int64_t getX() const;

    /**
     * Returns the Y component of the vector
     */
    int64_t getY() const;

    /**
     * Returns the Z component of the vector
     */
    int64_t getZ() const;

    /**
     * Sets the x, y and z components to the passed values.
     */
    void setAll(int64_t newx, int64_t newy, int64_t newz);

    /**
     * Calculates the distance between two points
     *
     * @warning the result is typecasted to a integer value
     * @returns distance to passed parameter
     */
    uint64_t getDistance(const Vector3d & origin) const;

    /**
     * Calculates the distance between two points, squared
     *
     * @returns distance to passed parameter, squared
     */
    double getDistanceSq(const Vector3d & origin) const;

    /**
     * Pack the vector into the passed frame.
     */
    void pack(Frame * frame) const;

    /**
     * Unpack the vector from the passed frame.
     */
    void unpack(InputFrame * frame);

  private:
    /// X-component value
    int64_t x;

    /// Y-component value
    int64_t y;

    /// Z-component value
    int64_t z;
};


#endif
