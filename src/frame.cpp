#include <string.h>
#include <netinet/in.h>
//#include <stdlib.h>
#include <string>

#include "logging.h"

#include "frame.h"

#define htonq(i)	( ((long long)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
#define ntohq		htonq

Frame::Frame()
{
	type = ft_Invalid;
	length = 0;
	data = NULL;
	unpackptr = 0;
}

Frame::Frame(Frame & rhs)
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
	char *packet = new char[length + 12];
	char *temp = packet;
	if (packet != NULL) {
		memcpy(temp, "TP01", 4);
		temp += 4;
		int ntype = htonl(type);
		int nlen = htonl(length);
		memcpy(temp, &ntype, 4);
		temp += 4;
		memcpy(temp, &nlen, 4);
		temp += 4;
		memcpy(temp, data, length);
	}
	return packet;
}

FrameType Frame::getType()
{
	return type;
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
	unpackptr = 0;
	int len;
	if (memcmp(newhead, "TP01", 4) == 0) {
		int ntype;
		int nlen;
		memcpy(&ntype, newhead + 4, 4);
		type = (FrameType) ntohl(ntype);
		memcpy(&nlen, newhead + 8, 4);
		len = ntohl(nlen);
	} else {
		len = -1;
	}
	if (len % 4 != 0)
		len = -1;
	if (type <= ft_Invalid || type >= ft_Max) {
		type = ft_Invalid;
		len = -1;
	}

	return len;
}

bool Frame::setType(FrameType nt)
{
	if (nt > ft_Invalid && nt < ft_Max) {
		type = nt;
		return true;
	} else {
		return false;
	}
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
	int padlen = 4 - len % 4;
	if (padlen == 4)
		padlen = 0;
	int netlen = htonl(len);
	char *temp = (char *) realloc(data, length + 4 + len + padlen);
	if (temp != NULL) {
		data = temp;
		temp += length;
		memcpy(temp, &netlen, 4);
		length += 4;
		temp += 4;
		memcpy(temp, str, len);
		length += len;
		temp += len;
		memset(temp, '\0', padlen);
		length += padlen;
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
	if (newoffset < length - 4 && newoffset >= 0) {
		unpackptr = newoffset;
	} else {
		return false;
	}
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
		int pad = 4 - len % 4;
		if (pad == 4)
			pad = 0;
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
