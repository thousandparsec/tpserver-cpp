#ifndef NOP_H
#define NOP_H

//class Order;
#include "order.h"

class Nop:public Order {
      public:
	Nop();
	~Nop();

	int getTime();
	void setTime(int time);

	void createFrame(Frame * f, int objID, int pos);
	void inputFrame(Frame * f);

	void createOutcome(Frame * f, int objID, int pos);

	static void describeOrder(int orderType, Frame * f);

      private:
	int waitTime;

};

#endif
