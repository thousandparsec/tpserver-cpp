/*  ListParameter baseclass
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
#include "orderparameters.h"

ListParameter::ListParameter(const std::string& aname, const std::string& adesc, Callback acallback) : OrderParameter(aname,adesc),  callback(acallback){
  id = opT_List;
}

void ListParameter::pack(Frame * f){
  Options options = callback();
  f->packInt(options.size());
  for(Options::iterator itcurr = options.begin();
      itcurr != options.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packString(itcurr->second.first);
    f->packInt(itcurr->second.second);
  }
 
  f->packIdMap(list);
}

bool ListParameter::unpack(Frame *f){
  if(!f->isEnoughRemaining(8))
    return false;
  int selsize = f->unpackInt(); // selectable list (should be zero)
  if(!f->isEnoughRemaining(4 + selsize * 12))
    return false;
  for(int i = 0; i < selsize; i++){
    f->unpackInt();
    std::string name = f->unpackStdString();
    if(!f->isEnoughRemaining(8 + (selsize - i - 1) * 12))
      return false;
    f->unpackInt(); 
  }
  
  list = f->unpackMap();
  return true;
}

StringParameter::StringParameter( const std::string& aname, const std::string& adesc ) : OrderParameter(aname,adesc) {
  id = opT_String;
}

void StringParameter::pack(Frame * f){
  f->packInt(1024);
  f->packString(string);
}

bool StringParameter::unpack(Frame *f){
  if(!f->isEnoughRemaining(8))
    return false;
  f->unpackInt();
  string = f->unpackStdString();
  return true;
}

TimeParameter::TimeParameter( const std::string& aname, const std::string& adesc, uint32_t time ) : OrderParameter(aname,adesc), turns(time) {
  id = opT_Time;
}

void TimeParameter::pack(Frame * f){
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

SpaceCoordParam::SpaceCoordParam( const std::string& aname, const std::string adesc ) :  OrderParameter(aname,adesc), position(){
  id = opT_Space_Coord_Abs;
}

void SpaceCoordParam::pack(Frame * f){
  position.pack(f);
}

bool SpaceCoordParam::unpack(Frame *f){
  if(!f->isEnoughRemaining(24))
    return false;
  position.unpack(f);
  return true;
}

ObjectOrderParameter::ObjectOrderParameter( const std::string& aname, const std::string& adesc ) : OrderParameter(aname,adesc), object(0){
  id = opT_Object_ID;
}

void ObjectOrderParameter::pack(Frame * f){
  f->packInt(object);
}

bool ObjectOrderParameter::unpack(Frame *f){
  if(f->isEnoughRemaining(4)){
    object = f->unpackInt();
    return true;
  }else{
    return false;
  }
}


