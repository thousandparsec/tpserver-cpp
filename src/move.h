#ifndef MOVE_H
#define MOVE_H

#include "order.h"

class Move:public Order {
      public:
	Move();
	virtual ~ Move();

	long long getX();
	long long getY();
	long long getZ();
	void setDest(long long x1, long long y1, long long z1);

	void createFrame(Frame * f, int objID, int pos);
	bool inputFrame(Frame * f);

	bool doOrder(IGObject * ob);

	static void describeOrder(int orderType, Frame * f);

      private:
	long long x;
	long long y;
	long long z;

};

#endif
