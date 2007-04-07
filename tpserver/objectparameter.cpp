/*  ObjectParameter baseclass
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

#include <stdlib.h>

#include "frame.h"

#include "objectparameter.h"

ObjectParameter::ObjectParameter() : type(0), name(), description(){
}

ObjectParameter::~ObjectParameter(){

}

uint32_t ObjectParameter::getType() const
{
  return type;
}

std::string ObjectParameter::getName() const{
  return name;
}

std::string ObjectParameter::getDescription() const{
  return description;
}

void ObjectParameter::setName(const std::string& nn){
  name = nn;
}

void ObjectParameter::setDescription(const std::string& nd){
  description = nd;
}

void ObjectParameter::packObjectDescFrame(Frame * f) const{

  f->packString(name);
  f->packInt(type);
  f->packString(description);
  
}

