#ifndef ORDER_H
#define ORDER_H

typedef enum {
	odT_Invalid = -1,
	odT_Nop = 0,
	odT_Move = 1,
	odT_Max
} OrderType;

typedef enum {
	opT_Invalid = -1,
	opT_Space_Coord = 0,
	opT_Time = 1,
	opT_Object_ID = 2,
	opT_Player_ID = 3,
	opT_Design_ID = 4,

	opT_Max
} OrderParamType;

class Frame;

class Order {

      public:
	Order();
	~Order();

	OrderType getType();

	void setType(OrderType ot);

	void createFrame(Frame * f, int objID, int pos);

	static void describeOrder(int ordertype, Frame * f);

      private:
	 Order(Order & rhs);
	Order operator=(Order & rhs);

	OrderType type;


};

#endif
