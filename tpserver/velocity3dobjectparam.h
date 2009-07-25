#ifndef VELOCITY3DOBJECTPARAM_H
#define VELOCITY3DOBJECTPARAM_H
/*  Velocity3dObjectParam class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/order.h>
#include <tpserver/orderparameters.h>

#include <tpserver/objectparameter.h>
#include <tpserver/vector3d.h>

class Velocity3dObjectParam : public ObjectParameter{

public:
  Velocity3dObjectParam();
  virtual ~Velocity3dObjectParam();

  virtual void packObjectFrame(Frame * f, uint32_t objID);
  virtual bool unpackModifyObjectFrame(Frame * f, uint32_t playerid);

  virtual ObjectParameter *clone() const;

  Vector3d getVelocity() const;
  void setVelocity(const Vector3d& nvel);
  
  uint32_t getRelative() const;
  void setRelative(uint32_t ob);

protected:
  Vector3d velocity;
  uint32_t relative;

};

#endif
