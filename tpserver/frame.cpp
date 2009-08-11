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
Frame::Frame(ProtocolVersion v)
  : version(v), type(ft_Invalid), typeversion(0),
    sequence(0), padstrings(false)
{
}

Frame::~Frame()
{
}

FrameType Frame::getType() const
{
  return type;
}

int Frame::getSequence() const
{
  return sequence;
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
  return data.length();
}


uint32_t Frame::getTypeVersion() const{
  return typeversion;
}

bool Frame::isPaddingStrings() const{
  return padstrings;
}


InputFrame::InputFrame(ProtocolVersion v, bool padding)
  : Frame(v), unpackptr(0)
{
  padstrings = padding;
}

int InputFrame::setHeader(const std::string& new_header)
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

void InputFrame::setData( const std::string& new_data )
{
  unpackptr = 0;
  // Actually the internal string reference count system prevents a copy here :)
  data = new_data;
}

bool InputFrame::isEnoughRemaining(uint32_t size) const{
  Logger::getLogger()->debug("isEnoughRemaining, checking for %d, have %d", size, data.length() - unpackptr);
  return (data.length() - unpackptr) >= size;
}

void InputFrame::advance( uint32_t amount )
{
  uint32_t newoffset = unpackptr + amount;
  if (newoffset < data.length() - 4)
    unpackptr = newoffset;
  //else throw
}

int InputFrame::unpackInt()
{
  int nval;
  memcpy(&nval, data.c_str() + unpackptr, 4);
  unpackptr += 4;
  return ntohl(nval);
}


std::string InputFrame::unpackString(){
  uint32_t len = unpackInt();
  if(unpackptr + len > data.length()){
    throw FrameException( fec_FrameError, "Not enough data to unpack string!" );
  }
  const char* pos = data.c_str() + unpackptr;
  unpackptr += len;

  if(padstrings){
    int pad = unpackptr % 4;
    if(pad != 0){
      if(unpackptr + (4 - pad) > data.length()){
        throw FrameException( fec_FrameError, "Not enough data to unpack string padding!" );
      }
      unpackptr += 4-pad;
    }
  }

  return std::string(pos, len);
}

int64_t InputFrame::unpackInt64()
{
  int64_t nval;
  memcpy(&nval, data.c_str() + unpackptr, 8);
  unpackptr += 8;
  return ntohq(nval);
}

char InputFrame::unpackInt8(){
  char rval = data.c_str()[unpackptr];
  unpackptr += 1;
  if(padstrings)
    unpackptr += 3;
  return rval;
}

IdMap InputFrame::unpackMap(){
  IdMap map;
  uint32_t msize = unpackInt();
  for(uint32_t i = 0; i < msize; ++i){
    uint32_t idx = unpackInt();
    map[idx] = unpackInt();
  }
  return map;
}

IdSet InputFrame::unpackIdSet(){
  IdSet set;
  uint32_t msize = unpackInt();
  for(uint32_t i = 0; i < msize; ++i){
    set.insert( unpackInt() );
  }
  return set;
}


OutputFrame::OutputFrame(ProtocolVersion v, bool padding)
  : Frame(v)
{
  padstrings = padding;
}

bool OutputFrame::setType(FrameType nt)
{
  if ((nt < ft_Invalid || (version == fv0_3 && nt > ft03_Max) || (version == fv0_4 && nt > ft04_Max)) && (nt < ftad_LogMessage || nt > ftad_Max)) // TODO - may want better admin checking here
    return false;

  type = nt;
  return true;
}

void OutputFrame::setSequence(int s) 
{
  sequence = s;
}

void OutputFrame::setTypeVersion(uint32_t tv){
  typeversion = tv;
}

void OutputFrame::setPadding(bool new_padding){
  padstrings = new_padding;
}

std::string OutputFrame::getPacket() const {
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

  int nlen = htonl(data.length());
  packet.append( (const char*)&nlen, 4 );

  // Body
  packet.append( data );

  return packet;
}


void OutputFrame::packString(const std::string &str){
  packInt(str.length());
  data.append( str );
    
  if ( padstrings ) {
    size_t pad = data.length() % 4;
    if ( pad != 0 ) data.append( 4-pad, '\0' );
  }
}


void OutputFrame::packInt(int val)
{
  int netval = htonl(val);
  data.append( (const char*) &netval, 4 );
}

void OutputFrame::packInt64(int64_t val)
{
  int64_t netval = htonq(val);
  data.append( (const char*) &netval, 8 );
}

void OutputFrame::packInt8(char val){
  data += val;
  if ( padstrings ) {
    data.append( 3, '\0' );
  }
}


void OutputFrame::packIdModList(const IdModList& modlist, uint32_t count, uint32_t from_position ){
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
}

void OutputFrame::packIdSet(const IdSet& idset)
{
  packInt(idset.size());
  for(IdSet::const_iterator idit = idset.begin(); idit != idset.end(); ++idit){
    packInt(*idit);
  }
}

void OutputFrame::packIdMap(const IdMap& idmap)
{
  packInt(idmap.size());
  for(IdMap::const_iterator idit = idmap.begin(); idit != idmap.end(); ++idit){
    packInt(idit->first);
    packInt(idit->second);
  }
}

void OutputFrame::packIdStringMap(const IdStringMap& idmap)
{
  packInt(idmap.size());
  for(IdStringMap::const_iterator idit = idmap.begin(); idit != idmap.end(); ++idit){
    packInt(idit->first);
    packString(idit->second);
  }
}

