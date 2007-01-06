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

TimeParameter::TimeParameter() : OrderParameter(), turns(0), max(0){
  type = opT_Time;
}

TimeParameter::~TimeParameter(){

}


void TimeParameter::packOrderFrame(Frame * f, uint32_t objID){
  f->packInt(turns);
  f->packInt(max);
}

bool TimeParameter::unpackFrame(Frame *f, unsigned int playerid){
  turns = f->unpackInt();
  f->unpackInt(); //read only max turns
  return true;
}

OrderParameter *TimeParameter::clone() const{
  TimeParameter* tp = new TimeParameter();
  tp->max = max;
  return tp;
}

uint32_t TimeParameter::getTime() const{
  return turns;
}

void TimeParameter::setTime(uint32_t time){
  turns = time;
}

uint32_t TimeParameter::getMax() const{
  return max;
}

void TimeParameter::setMax(uint32_t nmax){
  max = nmax;
}
