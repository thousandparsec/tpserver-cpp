/*  Frame class, the network packets for the TP procotol
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <string.h>
#include <netinet/in.h>
#include <string>
#include <cstdlib>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"

#include "frame.h"

#ifdef htonll
# define htonq		htonll
# define ntohq		ntohll
#endif

#ifndef htonq
#ifndef WORDS_BIGENDIAN
# define htonq(i)	( ((int64_t)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
#else
#define htonq(i)        i
#endif 
# define ntohq		htonq
#endif

// Default to creating version 3 frames
Frame::Frame()
{
  Frame::Frame(fv0_3);
}

Frame::Frame(ProtocolVersion v)
  : version(v), type(ft_Invalid), typeversion(0),
    sequence(0), length(0), data(NULL), padstrings(false),
    unpackptr(0)
{
}

Frame::~Frame()
{
  if (data != NULL)
    free(data);
}

std::string Frame::getPacket() const {
  std::string packet;
  packet.reserve( getLength() );
  packet += "TP";
  if( version <= 3 ){
    // Put in the version number
    for (int i = 100; i > 1; i = i / 10) {
      int digit = (version - (version / i * i)) / (i/10);
      packet += char( '0' + digit );
    }
  }else{
    //versions 4 and above
    packet += (char)(0xff & version);
    packet += (char)(0xff & typeversion);
  }
  int nseq = htonl(sequence);
  packet.append( (const char*)&nseq, 4 );

  int ntype = htonl(type);
  packet.append( (const char*)&ntype, 4 );

  int nlen = htonl(length);
  packet.append( (const char*)&nlen, 4 );

  // Body
  packet.append( data, length );

  return packet;
}

FrameType Frame::getType() const
{
  return type;
}

int Frame::getSequence() const
{
  return sequence;
}

bool Frame::setSequence(int s) 
{
  sequence = s;
  return true;
}

ProtocolVersion Frame::getVersion() const
{
  return version;
}

int Frame::getLength() const
{
  return getHeaderLength()+getDataLength();
}

int Frame::getHeaderLength() const
{
  return 16;
}

int Frame::getDataLength() const
{
  return length;
}

int Frame::setHeader(const std::string& new_header)
{
  const char* temp = new_header.data();

  unpackptr = 0;
  int len;

  if (memcmp(temp, "TP", 2) == 0) {
    temp += 2;

    if(version <= 3){
      char ver[] = {'\0','\0','\0'};
      memcpy(ver, temp, 2);
      int nversion = atoi(ver);
      version = (ProtocolVersion)nversion;
      temp += 2;
    }else{
      // version 4 and above
      version = (ProtocolVersion)(*temp);
      temp++;
      typeversion = (uint32_t)(*temp);
      temp++;
    }


    // pick up sequence number for versions greater than 02
    int nseq;
    memcpy(&nseq, temp, 4);
    sequence = ntohl(nseq);
    temp += 4;


    int ntype;
    memcpy(&ntype, temp, 4);
    type = (FrameType) ntohl(ntype);
    temp += 4;

    int nlen;
    memcpy(&nlen, temp, 4);
    len = ntohl(nlen);
    temp += 4;
  } else {
    len = -1;
  }

  return len;
}

bool Frame::setType(FrameType nt)
{
  if ((nt < ft_Invalid || (version == fv0_3 && nt > ft03_Max) || (version == fv0_4 && nt > ft04_Max)) && (nt < ftad_LogMessage || nt > ftad_Max)) // TODO - may want better admin checking here
    return false;

  type = nt;
  return true;
}

bool Frame::setData( const std::string& new_data )
{
  unpackptr = 0;
  if ( !new_data.empty() ) {
    char *temp = (char *) realloc(data, new_data.length() );
    if (temp != NULL) {
      data = temp;
      length = new_data.length();
      memcpy(data, new_data.data(), new_data.length() );
    } else {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

uint32_t Frame::getTypeVersion() const{
  return typeversion;
}

bool Frame::setTypeVersion(uint32_t tv){
  typeversion = tv;
  return true;
}

bool Frame::isPaddingStrings() const{
  return padstrings;
}

void Frame::enablePaddingStrings(bool on){
  padstrings = on;
}


bool Frame::packString(const std::string &str){
  int slen = str.length();
  if(!(packInt(slen))){
    throw new std::exception();
  }
  char *temp = (char *) realloc(data, length + slen + 3);
  if (temp != NULL) {
    data = temp;
    temp += length;

    // Actual string
    memcpy(temp, str.c_str(), slen);
    temp += slen;

    length += slen;

    if(padstrings){
      int pad = length % 4;
      if(pad != 0){
        for(int i = 0; i < 4-pad; i++){
          *temp = '\0';
          temp++;
        }
      }
    }
  }else{
    throw new std::exception();
  }
  return true;
}


bool Frame::packInt(int val)
{
  int netval = htonl(val);
  char *temp = (char *) realloc(data, length + 4);

  if (temp != NULL) {
    data = temp;
    temp += length;

    memcpy(temp, &netval, 4);
    length += 4;

  } else {
    return false;
  }
  return true;
}

bool Frame::packInt64(int64_t val)
{
  int64_t netval = htonq(val);
  char *temp = (char *) realloc(data, length + 8);

  if (temp != NULL) {
    data = temp;
    temp += length;

    memcpy(temp, &netval, 8);
    length += 8;

  } else {
    return false;
  }
  return true;
}

bool Frame::packInt8(char val){
  char *temp = (char *) realloc(data, length + 4);

  if (temp != NULL) {
    data = temp;

    data[length] = val;

    length += 1;
    if(padstrings){
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
    }
  } else {
    return false;
  }
  return true;
}


bool Frame::packIdModList(const IdModList& modlist, uint32_t count, uint32_t from_position ){
  if (count == 0) count = modlist.size();
  packInt(modlist.size() - count - from_position);
  packInt(count);
  IdModList::const_iterator itcurr = modlist.begin();
  if (from_position > 0 )
    std::advance(itcurr, from_position);
  for(uint32_t i = 0; i < count; i++, ++itcurr){
    packInt(itcurr->first);
    packInt64(itcurr->second);
  }
  return true;
}

bool Frame::packIdSet(const IdSet& idset)
{
  packInt(idset.size());
  for(IdSet::const_iterator idit = idset.begin(); idit != idset.end(); ++idit){
    packInt(*idit);
  }
  return true;
}

bool Frame::packIdMap(const IdMap& idmap)
{
  packInt(idmap.size());
  for(IdMap::const_iterator idit = idmap.begin(); idit != idmap.end(); ++idit){
    packInt(idit->first);
    packInt(idit->second);
  }
  return true;
}

bool Frame::packIdStringMap(const IdStringMap& idmap)
{
  packInt(idmap.size());
  for(IdStringMap::const_iterator idit = idmap.begin(); idit != idmap.end(); ++idit){
    packInt(idit->first);
    packString(idit->second);
  }
  return true;
}

bool Frame::isEnoughRemaining(uint32_t size) const{
  Logger::getLogger()->debug("isEnoughRemaining, checking for %d, have %d", size, length - unpackptr);
  return (length - unpackptr) >= size;
}

void Frame::advance( uint32_t amount )
{
  uint32_t newoffset = unpackptr + amount;
  if (newoffset < length - 4)
    unpackptr = newoffset;
  //else throw
}

int Frame::unpackInt()
{
  int nval;
  memcpy(&nval, data + unpackptr, 4);
  unpackptr += 4;
  return ntohl(nval);
}


std::string Frame::unpackStdString(){
  uint32_t len = unpackInt();
  if(unpackptr + len > length){
    throw new std::exception();
  }
  char cstr[len+1];

  memcpy(cstr, data + unpackptr, len);
  cstr[len] = '\0';
  unpackptr += len;
  if(padstrings){
    int pad = unpackptr % 4;
    if(pad != 0){
      if(unpackptr + (4 - pad) > length){
        throw new std::exception();
      }
      unpackptr += 4-pad;
    }
  }
  return std::string(cstr, 0, len);
}

int64_t Frame::unpackInt64()
{
  int64_t nval;
  memcpy(&nval, data + unpackptr, 8);
  unpackptr += 8;
  return ntohq(nval);
}

char Frame::unpackInt8(){
  char rval = data[unpackptr];
  unpackptr += 1;
  if(padstrings)
    unpackptr += 3;
  return rval;
}

IdMap Frame::unpackMap(){
  IdMap map;
  uint32_t msize = unpackInt();
  for(uint32_t i = 0; i < msize; ++i){
    uint32_t idx = unpackInt();
    map[idx] = unpackInt();
  }
  return map;
}

IdSet Frame::unpackIdSet(){
  IdSet set;
  uint32_t msize = unpackInt();
  for(uint32_t i = 0; i < msize; ++i){
    set.insert( unpackInt() );
  }
  return set;
}

