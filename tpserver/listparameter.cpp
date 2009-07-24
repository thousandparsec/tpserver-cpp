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
  id = opT_List;
}

ListParameter::~ListParameter(){

}


void ListParameter::packOrderFrame(Frame * f){
  std::map<uint32_t, std::pair<std::string, uint32_t> > options 
      = optionscallback.call();
  f->packInt(options.size());
  for(std::map<uint32_t, std::pair<std::string, uint32_t> >::iterator itcurr = options.begin();
      itcurr != options.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packString(itcurr->second.first);
    f->packInt(itcurr->second.second);
  }
 
  f->packIdMap(list);
}

bool ListParameter::unpackFrame(Frame *f, uint32_t playerid){
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

IdMap ListParameter::getList() const{
  return list;
}

void ListParameter::setList(IdMap nlist){
  list = nlist;
}

void ListParameter::setListOptionsCallback(ListOptionCallback cb){
  optionscallback = cb;
}
