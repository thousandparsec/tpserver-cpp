#ifndef FLEET_H
#define FLEET_H
/*  Fleet Object class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <map>
#include <list>

#include <tpserver/vector3d.h>
#include <tpserver/objectdata.h>

class Position3dObjectParam;
class Velocity3dObjectParam;
class SizeObjectParam;
class ReferenceObjectParam;
class RefQuantityListObjectParam;
class IntegerObjectParam;
class OrderQueueObjectParam;

class Fleet:public ObjectData {
      public:
	Fleet();
	virtual ~Fleet();
        
        Vector3d getPosition() const;
        Vector3d getVelocity() const;
        uint64_t getSize() const;
        void setPosition(const Vector3d & np);
        void setVelocity(const Vector3d & nv);
        void setSize(uint64_t ns);
        
        uint32_t getOwner() const;
        void setOwner(uint32_t no);

        void setDefaultOrderTypes();
	void addShips(int type, int number);
	bool removeShips(int type, int number);
	int numShips(int type);
	std::map<int, int> getShips() const;
	int totalShips() const;

	long long maxSpeed();
	std::list<int> firepower(bool draw);
	bool hit(std::list<int> firepower);

        int getDamage() const;
        void setDamage(int nd);

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	int getContainerType();

	ObjectData* clone() const;

      private:
        Position3dObjectParam * pos;
        Velocity3dObjectParam * vel;
        SizeObjectParam * size;
        ReferenceObjectParam * playerref;
	RefQuantityListObjectParam * shiplist;
	IntegerObjectParam * damage;
        OrderQueueObjectParam * orderqueue;

};

#endif
