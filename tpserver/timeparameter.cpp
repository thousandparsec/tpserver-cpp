/*  TimeParameter baseclass
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
#include <time.h>

#include "frame.h"

#include "timeparameter.h"

TimeParameter::TimeParameter( const std::string& aname, const std::string& adesc, uint32_t time ) : OrderParameter(aname,adesc), turns(time) {
  id = opT_Time;
}

TimeParameter::~TimeParameter(){

}


void TimeParameter::packOrderFrame(Frame * f){
  f->packInt(turns);
  f->packInt(1000);
}

bool TimeParameter::unpack(Frame *f){
  if(!f->isEnoughRemaining(8))
    return false;
  turns = f->unpackInt();
  f->unpackInt(); //read only max turns
  return true;
}

uint32_t TimeParameter::getTime() const{
  return turns;
}

void TimeParameter::setTime(uint32_t time){
  turns = time;
}

