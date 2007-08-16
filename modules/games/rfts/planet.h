#ifndef PLANET_H
#define PLANET_H
/*  Planet class
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

#include <tpserver/vector3d.h>

#include "staticobject.h"
#include "ownedobject.h"

class ReferenceObjectParam;
class ResourceListParam;
class OrderQueueObjectParam;
class IntegerObjectParam;

namespace RFTS_ {

class Planet : public StaticObject, public OwnedObject {

 public:
   Planet();
   virtual ~Planet();

   virtual ObjectData* clone() const;

   virtual void packExtraData(Frame * frame);
   virtual void doOnceATurn(IGObject* obj);
   virtual int getContainerType();


   uint32_t getOwner() const;
   void setOwner(uint32_t no);

   void setOrderTypes();

   void setDefaultResources();

   const uint32_t getCurrentRP() const;

   const std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources() const;

   const std::pair<uint32_t, uint32_t> getResource(uint32_t resTypeId) const;
   const std::pair<uint32_t, uint32_t> getResource(const std::string& resTypName) const;

   void setResource(uint32_t resTypeId, uint32_t currentVal = 0, uint32_t maxVal = 0);
   void setResource(const std::string& resType, uint32_t currentVal = 0, uint32_t maxVal = 0);
    
   void addResource(uint32_t resTypeId, uint32_t amount);
   void addResource(const std::string& resTypeName, uint32_t amount);

   bool removeResource(uint32_t resTypeId, uint32_t amount);
   bool removeResource(const std::string& resTypeName, uint32_t amount);

 private:

   void calcRP();
   void calcPopuation();
 
   ReferenceObjectParam * playerref;
   ResourceListParam* resources;
   OrderQueueObjectParam * orderqueue;

};


}

#endif
