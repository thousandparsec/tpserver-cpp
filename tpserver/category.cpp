/*  Category class
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

#include "design.h"
#include "frame.h"

#include "category.h"


Category::Category(){
  catid = 0;
}

Category::~Category(){
}

uint32_t Category::getCategoryId() const{
  return catid;
}

std::string Category::getName() const{
  return name;
}

std::string Category::getDescription() const{
  return desc;
}

void Category::packFrame(Frame* frame) const{
  frame->setType(ft03_Category);
  frame->packInt(catid);
  // TODO: is this really correct???
  frame->packInt64(0LL); //timestamp
  frame->packString(name.c_str());
  frame->packString(desc.c_str());
}

bool Category::doAddDesign(Design* d){
  return true;
}

bool Category::doModifyDesign(Design* d){
  return true;
}

void Category::setCategoryId(uint32_t c){
  catid = c;
}

void Category::setName(const std::string& n){
  name = n;
}

void Category::setDescription(const std::string& d){
  desc = d;
}

