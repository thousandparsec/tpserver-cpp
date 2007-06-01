#ifndef UNIVERSE_H
#define UNIVERSE_H
/*  Universe ObjectData class
 *
 *  Copyright (C) 2004, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/vector3d.h>
#include <tpserver/objectdata.h>

class IntegerObjectParam;
class Position3dObjectParam;
class SizeObjectParam;

class Universe:public ObjectData {
      public:
	Universe();
        virtual ~Universe();
        
        Vector3d getPosition() const;
        uint64_t getSize() const;
        void setPosition(const Vector3d & np);
        void setSize(uint64_t ns);

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	int getContainerType();

	ObjectData* clone();

	void setYear(int year);
	int getYear();

      private:
	Position3dObjectParam * pos;
        SizeObjectParam * size;
        IntegerObjectParam * year;

};

#endif