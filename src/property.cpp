/*  Property class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include <time.h>

#include "frame.h"

#include "property.h"

Property::Property(){
  propid = 0;
  timestamp = time(NULL);
}

Property::~Property(){

}

void Property::packFrame(Frame* frame) const{
  frame->setType(ft03_Property);
  frame->packInt(propid);
  frame->packInt64(timestamp);
  frame->packInt(1);
  frame->packInt(catid);
  frame->packInt(rank);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packString(tpcl_display.c_str());
}

unsigned int Property::getPropertyId() const{
  return propid;
}

unsigned int Property::getCategoryId() const{
  return catid;
}

unsigned int Property::getRank() const{
  return rank;
}

std::string Property::getName() const{
  return name;
}

std::string Property::getTpclDisplayFunction() const{
  return tpcl_display;
}

void Property::setPropertyId(unsigned int id){
  propid = id;
}

void Property::setCategoryId(unsigned int id){
  catid = id;
}

void Property::setRank(unsigned int r){
  rank = r;
}

void Property::setName(const std::string& n){
  name = n;
}

void Property::setDescription(const std::string& d){
  description = d;
}

void Property::setTpclDisplayFunction(const std::string& d){
  tpcl_display = d;
}
