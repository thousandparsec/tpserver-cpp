#include <string.h>
#include <netinet/in.h>
#include <string>

#include "logging.h"

#include "frame.h"

#ifdef htonll
# define htonq		htonll
# define ntohq		ntohll
#endif

#ifndef htonq
# define htonq(i)	( ((long long)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
# define ntohq		htonq
#endif

// Default to creating version 2 frames
Frame::Frame()
{
	Frame::Frame(fv0_2);
}

Frame::Frame(FrameVersion v)
{
	type = ft02_Invalid;
	length = 0;
	data = NULL;
	unpackptr = 0;
}

Frame::Frame(Frame &rhs)
{
	type = rhs.type;
	length = rhs.length;
	
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

Frame Frame::operator=(Frame & rhs)
{
	type = rhs.type;
	length = rhs.length;
	data = (char *) malloc(length);
	if (data != NULL) {
		memcpy(data, rhs.data, length);
	} else {
		type = ft_Invalid;
		length = 0;
	}
	unpackptr = 0;
}

char *Frame::getPacket()
{
	char *packet;
	char *temp;

	// Allocate the correct amount of memory
	if (version == fv0_1)
		packet = new char[length + 12];
	 else if (version == fv0_2)
		packet = new char[length + 16];
	temp = packet;

	if (packet != NULL) {
		// Header
		memcpy(temp, "TP", 4);
		temp += 2;

		// Put in the version number
		for (int i = 100; i > 1; i = i / 10) {
			int digit = (version - (version / i * i)) / (i/10);
			char v = '0' + digit;
			
			memcpy(temp, &v, 1);
			temp += 1;
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

FrameType Frame::getType()
{
	return type;
}

FrameVersion Frame::getVersion()
{
	return version;
}

int Frame::getLength()
{
	return length;
}

char *Frame::getData()
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

		char ver[] = {'\0','\0','\0'};
		memcmp(ver, temp, 2);
		int nversion = atoi(ver);
		version = (FrameVersion)nversion;
		temp += 2;
	
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

	if (version == fv0_1) {
		// Version 1 had to be word aligned
		if (len % 4 != 0)
			len = -1;
			
		if (type <= ft_Invalid || type >= ft_Max) {
			type = ft_Invalid;
			len = -1;
		}
	} else if (version == fv0_2) {
		if (type <= ft02_Invalid || type >= ft02_Max) {
			type = ft02_Invalid;
			len = -1;
		}
	}

	return len;
}

bool Frame::setType(FrameType nt)
{
	if (version == fv0_1)
		if (nt < ft_Invalid || nt > ft_Max)
			return false;
	else if (version == fv0_2)
		if (nt < ft02_Invalid || nt > ft02_Max)
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

bool Frame::packString(char *str)
{
	int len = strlen(str) + 1;

	// Version 0.1 needed to be correctly aligned
	int padlen = 0;
	if (version == fv0_1) {
		padlen = 4 - len % 4;
		if (padlen == 4)
			padlen = 0;
	}

	int netlen = htonl(len);
	char *temp = (char *) realloc(data, length + 4 + len + padlen);
	
	if (temp != NULL) {
		data = temp;
		temp += length;
		
		// Length
		memcpy(temp, &netlen, 4);
		temp += 4;
		
		// Actual string
		memcpy(temp, str, len);
		temp += len;
		
		// Pad the string
		memset(temp, '\0', padlen);

		length += 4 + len + padlen;
	} else {
		return false;
	}
	return true;
}

/*bool packString(std::string str){
  return packString(str.c_str());
  }
*/

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

int Frame::getUnpackOffset()
{
	return unpackptr;
}

bool Frame::setUnpackOffset(int newoffset)
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
		rtnstr = new char[len];
		memcpy(rtnstr, data + unpackptr, len);

		int pad = 0;
		if (version == fv0_1) {
			pad = 4 - len % 4;
			if (pad == 4)
				pad = 0;
		}
		
		unpackptr += len + pad;
	} else {
		Logger::getLogger()->debug("len < 0 or length < upackptr + len");
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

/*
long long Frame::unpackInt64(){
  long long rtn;
  int nval, nvalh, nvall;
  memcpy(&nval, data + unpackptr, 4);
  unpackptr += 4;
  nvalh = ntohl(nval);
  memcpy(&nval, data + unpackptr, 4);
  nvall = ntohl(nval);
  memcpy(&rtn, &nvall, 4);
  memcpy(&rtn + 4, &nvalh, 4);
  return rtn;
}*/

void Frame::createFailFrame(int code, char *reason)
{
	setType(ft_Fail);
	if (data != NULL) {
		free(data);
		length = 0;
		data = NULL;
		unpackptr = 0;
	}
	packInt(code);
	packString(reason);
}
