/*  Velocity3dObjectParam baseclass
 *
 *  Copyright (C) 2007 Lee Begg and the Thousand Parsec Project
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

#include "frame.h"

#include "velocity3dobjectparam.h"

Velocity3dObjectParam::Velocity3dObjectParam() : ObjectParameter(), velocity(), relative(0){
  type = obpT_Velocity;
}

Velocity3dObjectParam::~Velocity3dObjectParam(){

}


void Velocity3dObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  velocity.pack(f);
  f->packInt(relative);
}


bool Velocity3dObjectParam::unpackModifyObjectFrame(Frame *f, uint32_t playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(28))
    return false;
  //velocity.unpack(f);
  //relative = f->unpackInt();
  f->setUnpackOffset(f->getUnpackOffset() + 28);
  return true;
}

ObjectParameter* Velocity3dObjectParam::clone() const{
  return new Velocity3dObjectParam();
}

void Velocity3dObjectParam::setVelocity(const Vector3d &nvel){
  velocity = nvel;
}

Vector3d Velocity3dObjectParam::getVelocity() const{
  return velocity;
}

uint32_t Velocity3dObjectParam::getRelative() const{
  return relative;
}

void Velocity3dObjectParam::setRelative(uint32_t ob){
  relative = ob;
}

