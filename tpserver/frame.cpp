/*  Frame class, the network packets for the TP procotol
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
# define htonq(i)	( ((long long)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
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

Frame::Frame(FrameVersion v)
{
  type = ft_Invalid;
  typeversion = 0;
  
  length = 0;
  data = NULL;
  unpackptr = 0;
  version = v;
  padstrings = false;
}

Frame::Frame(const Frame &rhs)
{
	type = rhs.type;
        typeversion = rhs.typeversion;
	length = rhs.length;
	version = rhs.version;
	sequence = rhs.sequence;
        padstrings = rhs.padstrings;
	
	data = (char *) malloc(length);
	if (data != NULL) {
		memcpy(data, rhs.data, length);
	} else {
	  
	       type = ft_Invalid;
		length = 0;
	}
	unpackptr = 0;
}

Frame::~Frame()
{
	if (data != NULL)
		free(data);
}

Frame Frame::operator=(const Frame & rhs)
{
	type = rhs.type;
        typeversion = rhs.typeversion;
	length = rhs.length;
	version = rhs.version;
	sequence = rhs.sequence;
        padstrings = rhs.padstrings;
        
	data = (char *) malloc(length);
	if (data != NULL) {
		memcpy(data, rhs.data, length);
	} else {

	       type = ft_Invalid;
		length = 0;
	}
	unpackptr = 0;
	return *this;
}

char *Frame::getPacket() const{
  char *packet;
  char *temp;

  // Allocate the correct amount of memory
  packet = new char[getLength()];
  temp = packet;

  if (packet != NULL) {
    // Header
    memcpy(temp, "TP", 2);
    temp += 2;

    if(version <= 3){
      // Put in the version number
      for (int i = 100; i > 1; i = i / 10) {
        int digit = (version - (version / i * i)) / (i/10);
        char v = '0' + digit;
        
        memcpy(temp, &v, 1);
        temp += 1;
      }
    }else{
      //versions 4 and above
      *temp = (char)(0xff & version);
      temp++;
      *temp = (char)(0xff & typeversion);
      temp++;
    }
    
    // Sequence number is only present in version 2 and above
    if (version > 1) {
      int nseq = htonl(sequence);
      memcpy(temp, &nseq, 4);
      temp += 4;
    }
    
    int ntype = htonl(type);
    memcpy(temp, &ntype, 4);
    temp += 4;
    
    int nlen = htonl(length);
    memcpy(temp, &nlen, 4);
    temp += 4;
    
    // Body
    memcpy(temp, data, length);
  }
  
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

FrameVersion Frame::getVersion() const
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

char *Frame::getData() const
{
	char *tortn = (char *) malloc(length);
	if (tortn != NULL) {
		memcpy(tortn, data, length);
	}
	return tortn;
}

int Frame::setHeader(char *newhead)
{
  char* temp = newhead;

  unpackptr = 0;
  int len;
  
  if (memcmp(temp, "TP", 2) == 0) {
    temp += 2;

    if(version <= 3){
      char ver[] = {'\0','\0','\0'};
      memcpy(ver, temp, 2);
      int nversion = atoi(ver);
      version = (FrameVersion)nversion;
      temp += 2;
    }else{
      // version 4 and above
      version = (FrameVersion)(*temp);
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


  if (type <= ft_Invalid || (version == fv0_2 && type >= ft02_Max) || (version == fv0_3 && type >= ft03_Max)) {
    type = ft_Invalid;
    len = -1;
    
  }

  return len;
}

bool Frame::setType(FrameType nt)
{

  if (nt < ft_Invalid || (version == fv0_2 && nt > ft02_Max) || (version == fv0_3 && nt > ft03_Max))
    return false;
	
	type = nt;
	return true;
}

bool Frame::setData(char *newdata, int dlen)
{
	unpackptr = 0;
	if (dlen > 0) {
		char *temp = (char *) realloc(data, dlen);
		if (temp != NULL) {
			data = temp;
			length = dlen;
			memcpy(data, newdata, length);
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
}

bool Frame::isPaddingStrings() const{
  return padstrings;
}

void Frame::enablePaddingString(bool on){
  padstrings = on;
}

bool Frame::packString(const char *str)
{
  int slen = strlen(str);

  int netlen = htonl(slen);
  char *temp = (char *) realloc(data, length + 4 + slen + 3);
  
  if (temp != NULL) {
    data = temp;
    temp += length;
    
    // Length
    memcpy(temp, &netlen, 4);
    temp += 4;
    
    // Actual string
    memcpy(temp, str, slen);
    temp += slen;
    
    length += 4 + slen;
    
    if(padstrings){
      int pad = length % 4;
      if(pad != 0){
        for(int i = 0; i < 4-pad; i++){
          *temp = '\0';
          temp++;
        }
      }
    }
    
    
  } else {
    return false;
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

bool Frame::packInt64(long long val)
{
	long long netval = htonq(val);
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

bool Frame::packData(unsigned int len, char* bdata){
  char *temp = (char *) realloc(data, length + len + 3);
  
  if (temp != NULL) {
    data = temp;
    temp += length;
    memcpy(temp, bdata, len);
    length += len;
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
    return false;
  }
  return true;
}

bool Frame::isEnoughRemaining(uint32_t size) const{
  Logger::getLogger()->debug("isEnoughRemaining, checking for %d, have %d", size, length - unpackptr);
  return (length - unpackptr) >= size;
}

uint32_t Frame::getUnpackOffset() const
{
	return unpackptr;
}

bool Frame::setUnpackOffset(uint32_t newoffset)
{
	if (newoffset < length - 4 && newoffset >= 0)
		unpackptr = newoffset;
	else
		return false;
	
	return true;
}

int Frame::unpackInt()
{
	int nval;
	memcpy(&nval, data + unpackptr, 4);
	unpackptr += 4;
	return ntohl(nval);
}

char *Frame::unpackString()
{
	int len = unpackInt();
	char *rtnstr = NULL;
	
	if (len > 0 && length >= unpackptr + len) {
		rtnstr = new char[len + 1];
		memcpy(rtnstr, data + unpackptr, len);
		rtnstr[len] = '\0';
		unpackptr += len;
                if(padstrings){
                  int pad = unpackptr % 4;
                  if(pad != 0){
                    unpackptr += 4-pad;
                  }
                }
        }else if(len == 0){
            rtnstr = new char[1];
            rtnstr[0] = '\0';
	} else {
        Logger::getLogger()->debug("len < 0 or length < upackptr + len, len = %d, length = %d", len, length);
	}
	//printf("unpackptr %d\n", unpackptr);
	return rtnstr;
}

long long Frame::unpackInt64()
{
	long long nval;
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

void Frame::unpackData(unsigned int len, char* bdata){
  Logger::getLogger()->warning("Using unpackData, might not be safe");
  
  memcpy(bdata, data + unpackptr, len);
  unpackptr += len;
  if(padstrings){
    int pad = unpackptr % 4;
    if(pad != 0){
      unpackptr += 4-pad;
    }
  }
}

void Frame::createFailFrame(FrameErrorCode code, char *reason)
{
  
    setType(ft02_Fail);

  if (data != NULL) {
    free(data);
    length = 0;
    data = NULL;
    unpackptr = 0;
  }
  packInt(code);
  packString(reason);
  
}
