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
	opT_Space_Coord_Abs = 0,
	opT_Time = 1,
	opT_Object_ID = 2,
	opT_Player_ID = 3,
	opT_Space_Coord_Rel = 4,
	opT_Range = 5,

	opT_Max
} OrderParamType;

class Frame;

class Order {

      public:

	OrderType getType();

	virtual void createFrame(Frame * f, int objID, int pos);
	virtual void inputFrame(Frame * f);



	static void describeOrder(int ordertype, Frame * f);
	static Order *createOrder(OrderType ordertype);

      protected:
	 OrderType type;


};

#endif
