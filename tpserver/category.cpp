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

#include "category.h"


Category::Category() : ProtocolObject(ft03_Category,0,"","") {
}

Category::Category( const std::string& nname, const std::string& ndesc ) : ProtocolObject(ft03_Category,0,nname,ndesc) {
}

Category::~Category(){
}

uint32_t Category::getCategoryId() const{
  return getId();
}

void Category::pack(OutputFrame::Ptr frame) const{
  frame->setType(ft03_Category);
  frame->packInt(id);
  frame->packInt64(getModTime()); //timestamp
  frame->packString(name);
  frame->packString(desc);
}

bool Category::doAddDesign(Design::Ptr d){
  return true;
}

bool Category::doModifyDesign(Design::Ptr d){
  return true;
}

void Category::setCategoryId(uint32_t c){
  setId( c );
}


