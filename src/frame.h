#ifndef FRAME_H
#define FRAME_H

typedef enum {
	fv0_1 = 1,
	fv0_2 = 2,
	fv_Max,
} FrameVersion;

typedef enum {
// New Frame Codes
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
	//only for future reference
  	ft02_Board_Get = 16,
  	ft02_Board = 17,
  	ft02_Message_Get = 18,
  	ft02_Message = 19,
  	ft02_Message_Post = 20,
	ft02_Max,


} FrameType;

typedef enum {
  fec_Invalid = -1,
  fec_ProtocolError = 0,
  fec_FrameError = 1,
  fec_PermUnavailable = 2,
  fec_TempUnavailable = 3,
  fec_NonExistant = 4,
  fec_Max
} FrameErrorCode;


//class std::string;

class Frame {

	public:
		Frame();
		Frame(FrameVersion v);
		Frame(Frame &rhs);

		~Frame();

		Frame operator=(Frame &rhs);

		int setHeader(char *newhead);
		char *getPacket();
		int getHeaderLength();		// The length of the header section
		int getDataLength();		// The length of the data section
		int getLength();			// The total length of the packet
		
		// Data
		char *getData();
		bool setData(char *newdata, int dlen);

		// Type
		FrameType getType();
		bool setType(FrameType nt);
		
		// Sequence
		int getSequence();
		bool setSequence(int s);
		
		// Version
		FrameVersion getVersion();
		
		bool packString(char *str);
		//bool packString(std::string str);
		bool packInt(int val);
		bool packInt64(long long val);

		// uses these functions with care
		int getUnpackOffset();
		bool setUnpackOffset(int newoffset);

		int unpackInt();
		char *unpackString();
		long long unpackInt64();

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
