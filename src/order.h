#ifndef ORDER_H
#define ORDER_H

// inbuilt orders only
typedef enum {
	odT_Invalid = -1,
	odT_Nop = 0,
	odT_Move = 1,
	odT_Build = 2,
	odT_Colonise = 3,
	odT_Fleet_Split = 4,
	odT_Fleet_Merge = 5,
	odT_Max
} OrderType;

typedef enum {
	opT_Invalid = -1,
	opT_Space_Coord_Abs = 0,
	opT_Time = 1,
	opT_Object_ID = 2,
	opT_Player_ID = 3,
	opT_Space_Coord_Rel = 4,
	opT_Range = 5,
	opT_List = 6,
	opT_String = 7,

	opT_Max
} OrderParamType;

class Frame;
class IGObject;

class Order {

      public:

	int getType() const;
	void setType(int ntype);

	virtual void createFrame(Frame * f, int objID, int pos);
	virtual bool inputFrame(Frame * f);

	virtual bool doOrder(IGObject * ob) = 0;

	virtual void describeOrder(Frame * f) const;
	virtual Order *clone() const = 0;

      protected:
	 int type;


};

#endif
