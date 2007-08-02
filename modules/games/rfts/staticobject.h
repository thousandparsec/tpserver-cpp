#ifndef staticobject_H
#define staticobject_H
/*  static object class
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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


#include <string>

#include <tpserver/vector3d.h>
#include <tpserver/objectdata.h>

class Position3dObjectParam;
class SizeObjectParam;

namespace RFTS_ {

class StaticObject:public ObjectData {
 public:
   StaticObject();
   virtual ~StaticObject() {};
        
   Vector3d getPosition() const;
   void setPosition(const Vector3d & np);

   uint64_t getSize() const;
   void setSize(uint64_t ns);
   
   virtual void packExtraData(Frame * frame);
   virtual void doOnceATurn(IGObject * obj);
   virtual int getContainerType();

   virtual ObjectData* clone() const;
   
   void setTypeName(const std::string& n);
   void setTypeDescription(const std::string& d);
   
 private:
   Position3dObjectParam * pos;
   SizeObjectParam * size;

};

}

#endif
