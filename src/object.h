#ifndef OBJECT_H
#define OBJECT_H
/*  In Game Object class
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

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

	bool setID(unsigned int newid);
	void autoSetID();
	void setType(unsigned int newtype);
	void setSize(unsigned long long newsize);
	void setName(char *newname);
	void setPosition(const Vector3d & npos);
	void setFuturePosition(const Vector3d & npos);
	void updatePosition();
	void setVelocity(const Vector3d & nvel);
	
	void removeFromParent();

	int getContainerType();
	std::set<unsigned int> getContainedObjects();
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
	Vector3d futurepos;
	Vector3d vel;
	
	unsigned int parentid;


	 std::set < unsigned int >children;

	 std::list < Order * >orders;

	 ObjectData *myObjectData;

};

#endif
