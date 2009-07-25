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

ListParameter::ListParameter(const std::string& aname, const std::string& adesc, Callback acallback) : OrderParameter(aname,adesc),  callback(acallback){
  id = opT_List;
}

ListParameter::~ListParameter(){

}


void ListParameter::packOrderFrame(Frame * f){
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

IdMap ListParameter::getList() const{
  return list;
}

void ListParameter::setList(IdMap nlist){
  list = nlist;
}

