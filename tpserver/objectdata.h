#ifndef OBJECTDATA_H
#define OBJECTDATA_H
/*  ObjectData base class
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

#include <stdint.h>
#include <list>
#include <string>

class Frame;
class IGObject;
class ObjectParameter;
class ObjectParameterGroup;

class ObjectData {

      public:
	ObjectData();
	virtual ~ObjectData();
        
        std::string getTypeName() const;

	virtual void packExtraData(Frame * frame);
        void packObjectParameters(Frame* frame, uint32_t playerid);
        bool unpackModifyObject(Frame* frame, uint32_t playerid);
        
        void packObjectDescFrame(Frame* frame);
        
        ObjectParameter* getParameterByType(uint32_t ptype);

	virtual void doOnceATurn(IGObject * obj) = 0;

	virtual int getContainerType() = 0;

	virtual ObjectData* clone() const = 0;

	void touchModTime();
	long long getModTime() const;
        bool isDirty() const;
        //persistence only
        void setModTime(uint64_t time);
        void setDirty(bool nd);
        //object only
        void signalRemoval();

      protected:
        std::list<ObjectParameterGroup*> paramgroups;
        std::string nametype;
        std::string typedesc;

      private:
	long long modtime;
        bool dirty;

};

#endif
