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
  	ft02_Connect = 2,
  	ft02_Login = 3,
  	ft02_Object_Get = 4,
  	ft02_Object_GetByPos = 5,
  	ft02_Object = 6,
  	ft02_OrderDesc_Get = 7,
  	ft02_OrderDesc = 8,
  	ft02_Order_Get = 9,
  	ft02_Order = 10,
  	ft02_Order_Add = 11,
  	ft02_Order_Remove = 12,
  	ft02_Board_Get = 13,
  	ft02_Board = 14,
  	ft02_Message_Get = 15,
  	ft02_Message = 16,
  	ft02_Message_Post = 17,
	ft02_Max,
	
// Old Frame Codes
	ft_Invalid = -1,
	ft_Connect = 0,
	ft_OK = 1,
	ft_Login = 2,
	ft_Fail = 3,
	ft_Get_Object = 4,
	ft_Object = 5,
	ft_Get_Order = 6,
	ft_Order = 7,
	ft_Add_Order = 8,
	ft_Remove_Order = 9,
	ft_Describe_Order = 10,
	ft_Order_Description = 11,
	ft_Get_Outcome = 12,
	ft_Outcome = 13,
	ft_Max
} FrameType;

//class std::string;

class Frame {

	public:
		Frame();
		Frame(FrameVersion v);
		Frame(Frame &rhs);

		~Frame();

		Frame operator=(Frame &rhs);

		char *getPacket();
		FrameType getType();
		int getLength();
		char *getData();

		int setHeader(char *newhead);
		bool setType(FrameType nt);
		bool setData(char *newdata, int dlen);

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

		void createFailFrame(int code, char *reason);

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
