/*  Stores Binary data to be sent to the client.
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

#include "frame.h"

#include "datachunk.h"

unsigned int DataChunk::nextid = 1;

DataChunk::DataChunk(){
  did = nextid++;
  bdata = NULL;
}

DataChunk::DataChunk(const DataChunk &rhs){
  did = nextid++;
  pid = rhs.pid;
  turn = rhs.turn;
  mimetype = rhs.mimetype;
  length = rhs.length;
  bdata = new char[length];
  memcpy(bdata, rhs.bdata, length);
}

DataChunk::~DataChunk(){
  if(bdata != NULL)
    delete[] bdata;
}

unsigned int DataChunk::getId() const{
  return did;
}

unsigned int DataChunk::getOwner() const{
  return pid;
}

void DataChunk::setOwner(unsigned int playerid){
  pid = playerid;
}

void DataChunk::setTurnCreated(unsigned int cturn){
  turn = cturn;
}

void DataChunk::setMimeType(const std::string mtype){
  mimetype = mtype;
}

void DataChunk::setData(unsigned int len, char* data){
  if(bdata != NULL)
    delete[] bdata;
  
  length = len;
  bdata = new char[len];
  memcpy(bdata, data, len);
}

void DataChunk::packHeader(Frame* f) const{
  f->packInt(did);
  f->packInt(turn);
  f->packString(mimetype.c_str());
  f->packInt(length);
}

void DataChunk::packData(Frame* f) const{
  
}
