#ifndef OBJECT_H
#define OBJECT_H

#include <list>
#include <set>
#include <map>

// must try to fix this

#include "order.h"

class Frame;
class Game;
//class Order;


class IGObject {

      public:
	IGObject();
	IGObject(IGObject & rhs);
	~IGObject();

	IGObject operator=(IGObject & rhs);

	unsigned int getID();
	unsigned int getType();
	unsigned long long getSize();
	char *getName();
	long long getPositionX();
	long long getPositionY();
	long long getPositionZ();
	long long getVelocityX();
	long long getVelocityY();
	long long getVelocityZ();
	long long getAccelerationX();
	long long getAccelerationY();
	long long getAccelerationZ();
	 std::set < unsigned int >getContainedObjects();

	bool setID(unsigned int newid);
	void autoSetID();
	void setType(unsigned int newtype);
	void setSize(unsigned long long newsize);
	void setName(char *newname);
	void setPosition3(long long x, long long y, long long z);
	void setVelocity3(long long x, long long y, long long z);
	void setAcceleration3(long long x, long long y, long long z);

	bool addContainedObject(unsigned int addObjectID);
	bool removeContainedObject(unsigned int removeObjectID);

	bool addOrder(Order * ord, int pos, int playerid);
	bool removeOrder(int pos, int playerid);
	Order *getOrder(int pos, int playerid);
	int getNumOrders(int playerid);

	bool addAction(int currpid, int newpid, OrderType ot);
	bool removeAction(int currpid, int newpid, OrderType ot);
	 std::set < OrderType > getActions(int currpid, int newpid);


	void createFrame(Frame * frame, int playerid);

      protected:
	static Game *myGame;

      private:

	static unsigned int nextAutoID;

	unsigned int id;
	unsigned int type;
	unsigned long long size;
	char *name;
	long long posx;
	long long posy;
	long long posz;
	long long velx;
	long long vely;
	long long velz;
	long long accx;
	long long accy;
	long long accz;

	 std::set < unsigned int >children;

	 std::list < Order * >orders;
	 std::map < int, std::set < OrderType > >actions;

};

#endif
