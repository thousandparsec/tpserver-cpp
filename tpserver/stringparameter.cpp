/*  StringParameter baseclass
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

#include "stringparameter.h"

StringParameter::StringParameter() : OrderParameter(), string(), max(1024){
  type = opT_String;
}

StringParameter::~StringParameter(){

}


void StringParameter::packOrderFrame(Frame * f){
  f->packInt(max);
  f->packString(string.c_str());
}

bool StringParameter::unpackFrame(Frame *f, unsigned int playerid){
  if(!f->isEnoughRemaining(8))
    return false;
  f->unpackInt();
  char* ts = f->unpackString();
  if(ts != NULL){
    string = std::string(ts);
    return true;
  }else{
    return false;
  }
}

OrderParameter *StringParameter::clone() const{
  StringParameter* sp = new StringParameter();
  sp->max = max;
  return sp;
}

std::string StringParameter::getString() const{
  return string;
}

void StringParameter::setString(const std::string& rhs){
  string = rhs;
}

uint32_t StringParameter::getMax() const{
  return max;
}

void StringParameter::setMax(uint32_t nmax){
  max = nmax;
}

