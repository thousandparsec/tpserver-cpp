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

#include "listparameter.h"

ListParameter::ListParameter() : OrderParameter(), list(), optionscallback(){
  type = opT_List;
}

ListParameter::~ListParameter(){

}


void ListParameter::packOrderFrame(Frame * f, uint32_t objID, uint32_t playerid){
  std::map<uint32_t, std::pair<std::string, uint32_t> > options 
      = optionscallback.call(objID, playerid);
  f->packInt(options.size());
  for(std::map<uint32_t, std::pair<std::string, uint32_t> >::iterator itcurr = options.begin();
      itcurr != options.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packString(itcurr->second.first.c_str());
    f->packInt(itcurr->second.second);
  }
  
  f->packInt(list.size());
  for(std::map<uint32_t,uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
}

bool ListParameter::unpackFrame(Frame *f, unsigned int playerid){
  int selsize = f->unpackInt(); // selectable list (should be zero)
  for(int i = 0; i < selsize; i++){
    f->unpackInt();
    delete[] (f->unpackString());
    f->unpackInt(); 
  }
  
  for(int i = f->unpackInt(); i > 0; i--){
    uint32_t key = f->unpackInt();
    uint32_t value = f->unpackInt();
    list[key] = value;
  }
  
  return true;
}

OrderParameter *ListParameter::clone() const{
  return new ListParameter();
}

std::map<uint32_t,uint32_t> ListParameter::getList() const{
  return list;
}

void ListParameter::setList(std::map<uint32_t,uint32_t> nlist){
  list = nlist;
}

void ListParameter::setListOptionsCallback(ListOptionCallback cb){
  optionscallback = cb;
}
