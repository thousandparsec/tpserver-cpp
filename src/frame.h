#ifndef FRAME_H
#define FRAME_H
/*  TP protocol Frame class
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

typedef enum {
	fv0_1 = 1,
	fv0_2 = 2,
	fv0_3 = 3,
	fv_Max,
} FrameVersion;

typedef enum {
// New Frame Codes
	ft_Invalid = -1,
	ft02_Invalid = -1,
	ft02_OK = 0,
  	ft02_Fail = 1,
	ft02_Sequence = 2,
  	ft02_Connect = 3,
  	ft02_Login = 4,
  	ft02_Object_GetById = 5,
	ft02_Object_GetByPos = 6,
  	ft02_Object = 7,
  	ft02_OrderDesc_Get = 8,
  	ft02_OrderDesc = 9,
  	ft02_Order_Get = 10,
  	ft02_Order = 11,
  	ft02_Order_Insert = 12,
  	ft02_Order_Remove = 13,
	ft02_Time_Remaining_Get = 14,
	ft02_Time_Remaining = 15,
  	ft02_Board_Get = 16,
  	ft02_Board = 17,
  	ft02_Message_Get = 18,
  	ft02_Message = 19,
  	ft02_Message_Post = 20,
	ft02_Message_Remove = 21,
	ft02_ResDesc_Get = 22,
	ft02_ResDesc = 23,
	ft02_Max = 24,
	//Version 3 additions follow
	ft03_Redirect = 24,
	ft03_Features_Get = 25,
	ft03_Features = 26,
	ft03_Ping = 27,
	ft03_Order_Probe = 28,
	//Design category and component
	ft03_Data_URL_Get = 35,
	ft03_Data_URL = 36,
	ft03_Player_Get = 37,
	ft03_Player = 38,
	ft03_Board_List_Get = 39,
	ft03_Board_List = 40,
	ft03_ResDesc_List_Get = 41,
	ft03_ResDesc_List = 42,
	ft03_Max,


} FrameType;

typedef enum {
  fec_Invalid = -1,
  fec_ProtocolError = 0,
  fec_FrameError = 1,
  fec_PermUnavailable = 2,
  fec_TempUnavailable = 3,
  fec_NonExistant = 4,
  fec_PermissionDenied = 5,
  fec_Max
} FrameErrorCode;


//class std::string;

class Frame {

	public:
		Frame();
		Frame(FrameVersion v);
		Frame(const Frame &rhs);

		~Frame();

		Frame operator=(const Frame &rhs);

		int setHeader(char *newhead);
		char *getPacket() const;
		int getHeaderLength() const;		// The length of the header section
		int getDataLength() const;		// The length of the data section
		int getLength() const;			// The total length of the packet
		
		// Data
		char *getData() const;
		bool setData(char *newdata, int dlen);

		// Type
		FrameType getType() const;
		bool setType(FrameType nt);
		
		// Sequence
		int getSequence() const;
		bool setSequence(int s);
		
		// Version
		FrameVersion getVersion() const;
		
		bool packString(const char *str);
		//bool packString(std::string str);
		bool packInt(int val);
		bool packInt64(long long val);
		bool packInt8(char val);
		bool packData(unsigned int len, char* bdata);

		// uses these functions with care
		int getUnpackOffset() const;
		bool setUnpackOffset(int newoffset);

		int unpackInt();
		char *unpackString();
		long long unpackInt64();
		char unpackInt8();
		void unpackData(unsigned int len, char* bdata);

		void createFailFrame(FrameErrorCode code, char *reason);

	private:
		FrameVersion version;
		FrameType type;
	
		// Which packet sequence does this refer to?
		int sequence;
		
		// Frame length
		int length;

		// Actual data of the frame
		char *data;

		int unpackptr;
};

#endif
