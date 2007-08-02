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

#include "emptyobject.h"

class ReferenceObjectParam;
class ResourceListObjectParam;
class OrderQueueObjectParam;
class IntegerObjectParam;

namespace RFTS_ {

class Planet : public EmptyObject {

 public:
   Planet();
   virtual ~Planet();

   virtual ObjectData* clone() const;

   virtual void packExtraData(Frame * frame);
   virtual void doOnceATurn(IGObject* obj);
   virtual int getContainerType();

   void setDefaultOrderTypes();
           
   uint32_t getOwner() const;
   void setOwner(uint32_t no);

   std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources();
   uint32_t getResource(uint32_t restype) const;
    
   void setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress);
   void addResource(uint32_t restype, uint32_t amount);
   bool removeResource(uint32_t restype, uint32_t amount);

 private:
   ReferenceObjectParam * playerref;
   ResourceListObjectParam* resources;
   OrderQueueObjectParam * orderqueue;

   IntegerObjectParam *resourcePoints;
};


}

#endif
