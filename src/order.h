#ifndef ORDER_H
#define ORDER_H

typedef enum {
	odT_Invalid = -1,
	odT_Nop = 0,
	odT_Move = 1,
	odT_Max
} OrderType;

class Frame;

class Order {

      public:
	Order();
	~Order();

	OrderType getType();

	void setType(OrderType ot);

	void createFrame(Frame * f, int objID, int pos);

      private:
	 Order(Order & rhs);
	Order operator=(Order & rhs);

	OrderType type;


};

#endif
