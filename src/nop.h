#ifndef NOP_H
#define NOP_H

//class Order;
#include "order.h"

class Nop:public Order {
      public:
	Nop();
	virtual ~Nop();

	int getTime();
	void setTime(int time);

	void createFrame(Frame * f, int objID, int pos);
	bool inputFrame(Frame * f);

	bool doOrder(IGObject * ob);

	void describeOrder(Frame * f) const;
	Order* clone() const;

      private:
	int waitTime;

};

#endif
