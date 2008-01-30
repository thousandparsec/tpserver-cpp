/*  Empty Object class
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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


#include "emptyobject.h"

EmptyObjectType::EmptyObjectType() : SpaceObjectType(){
}

EmptyObjectType::~EmptyObjectType(){
}

void EmptyObjectType::setTypeName(const std::string& n){
  nametype = n;
}

void EmptyObjectType::setTypeDescription(const std::string& d){
  typedesc = d;
}

ObjectBehaviour* EmptyObjectType::createObjectBehaviour() const{
  return new EmptyObject();
}

EmptyObject::EmptyObject() : SpaceObject(){
}

void EmptyObject::doOnceATurn(){
}


int EmptyObject::getContainerType(){
  return 1;
}
