#ifndef OBJECTTYPE_H
#define OBJECTTYPE_H
/*  ObjectType base class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <map>
#include <string>

class Frame;
class IGObject;
class ObjectParameterGroupDesc;

class ObjectType {

      public:
	ObjectType();
	virtual ~ObjectType();
        
        std::string getTypeName() const;
        long long getModTime() const;
        //only for TP02/03 support
	virtual void packExtraData(Frame * frame, IGObject* obj);
        
        void packObjectDescFrame(Frame* frame);

	virtual void doOnceATurn(IGObject * obj) = 0;

	virtual int getContainerType() = 0;

	virtual void setupObject(IGObject* obj) const;

        //object only
        
        void signalRemoval(IGObject* obj);

      protected:
        void addParameterGroupDesc(ObjectParameterGroupDesc* group);

        std::string nametype;
        std::string typedesc;

      private:
        void touchModTime();
	long long modtime;
        uint32_t nextparamgroupid;
        std::map<uint32_t, ObjectParameterGroupDesc*> paramgroups;
};

#endif
