/*  Stores and provides access to DataChunks.
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

#include "datachunk.h"
#include "logging.h"
#include "frame.h"

#include "datastore.h"

DataStore::DataStore(){

}

DataStore::~DataStore(){
  while(!store.empty()){
    delete store.begin()->second;
    store.erase(store.begin());
  }
}


void DataStore::getData(unsigned int dataid, Frame* frame, int pid){
  // find datachunk
  std::map<unsigned int, DataChunk*>::iterator itcurr = store.find(dataid);
  if(itcurr != store.end()){

    DataChunk* chunk = itcurr->second;

    // check owner
    if(pid == (int)chunk->getOwner()){
      
      frame->setType(ft03_Data_URL);
      chunk->pack(frame);

      
    }else{
      Logger::getLogger()->warning("A player tried to get a DataChunk that wasn't for them.");
      frame->createFailFrame(fec_PermissionDenied, "It's not for you");
    }
  }else{
    Logger::getLogger()->warning("A player tried to get a DataChunk that doesn't exist");
    frame->createFailFrame(fec_NonExistant, "Chunk does not exist");
  }

}

void DataStore::removeData(unsigned int dataid, Frame* frame, int pid){
  // find datachunk
  std::map<unsigned int, DataChunk*>::iterator itcurr = store.find(dataid);
  if(itcurr != store.end()){
    
    DataChunk* chunk = itcurr->second;
    
    // check owner
    if(pid == (int)chunk->getOwner()){
 
      //remove chunk
      delete chunk;
      store.erase(itcurr);
      frame->setType(ft02_OK);
      frame->packString("Removed Data ok");
      
    }else{
      Logger::getLogger()->warning("A player tried to remove a DataChunk that wasn't for them.");
      frame->createFailFrame(fec_PermissionDenied, "It's not for you");
    }
  }else{
    Logger::getLogger()->warning("A player tried to remove a DataChunk header that doesn't exist");
    frame->createFailFrame(fec_NonExistant, "Chunk does not exist");
  }
}

void DataStore::addData(DataChunk* dc){
  store[dc->getId()] = dc;
}
