#ifndef OBJECT_H
#define OBJECT_H

#include <list>
#include <set>
#include <map>

// must try to fix this

#include "order.h"
#include "vector3d.h"

class Frame;
class Game;
class ObjectData;
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
	Vector3d getPosition();
	Vector3d getVelocity();
	unsigned int getParent();
	
	std::set<unsigned int> getContainedObjects();

	bool setID(unsigned int newid);
	void autoSetID();
	void setType(unsigned int newtype);
	void setSize(unsigned long long newsize);
	void setName(char *newname);
	void setPosition(const Vector3d & npos);
	void setVelocity(const Vector3d & nvel);
	
	void removeFromParent();

	bool addContainedObject(unsigned int addObjectID);
	bool removeContainedObject(unsigned int removeObjectID);

	bool addOrder(Order * ord, int pos, int playerid);
	bool removeOrder(unsigned int pos, int playerid);
	Order *getOrder(int pos, int playerid);
	int getNumOrders(int playerid);

	Order * getFirstOrder();
	void removeFirstOrder();



	void createFrame(Frame * frame, int playerid);

	ObjectData* getObjectData();

      protected:
	static Game *myGame;

      private:

	static unsigned int nextAutoID;

	unsigned int id;
	unsigned int type;
	unsigned long long size;
	char *name;
	Vector3d pos;
	Vector3d vel;
	
	unsigned int parentid;


	 std::set < unsigned int >children;

	 std::list < Order * >orders;

	 ObjectData *myObjectData;

};

#endif
