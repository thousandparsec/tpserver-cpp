/*  Position3dObjectParam baseclass
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

#include "position3dobjectparam.h"

Position3dObjectParam::Position3dObjectParam() : ObjectParameter(), position(), relative(0){
  type = obpT_Position_3D;
}

Position3dObjectParam::~Position3dObjectParam(){

}


void Position3dObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  position.pack(f);
  f->packInt(relative);
}


bool Position3dObjectParam::unpackModifyObjectFrame(InputFrame *f, uint32_t playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(28))
    return false;
  //position.unpack(f);
  //relative = f->unpackInt();
  f->advance(28);
  return true;
}

ObjectParameter* Position3dObjectParam::clone() const{
  return new Position3dObjectParam();
}

void Position3dObjectParam::setPosition(const Vector3d &npos){
  position = npos;
}

Vector3d Position3dObjectParam::getPosition() const{
  return position;
}

uint32_t Position3dObjectParam::getRelative() const{
  return relative;
}

void Position3dObjectParam::setRelative(uint32_t ob){
  relative = ob;
}

