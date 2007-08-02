#ifndef fleet_H
#define fleet_H
/*  fleet class
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

#include "emptyobject.h"

class Velocity3dObjectParam;
class ReferenceObjectParam;
class RefQuantityListObjectParam;
class IntegerObjectParam;
class OrderQueueObjectParam;

namespace RFTS_ {

class Fleet : public EmptyObject
{
 public:
   Fleet();
   virtual ~Fleet() {}

   uint32_t getOwner() const;
   void setOwner(uint32_t no);

   int getDamage() const;
   void setDamage(int nd);
   void takeDamage(int dmg);
   
   void setDefaultOrderTypes();
   void addShips(int type, uint32_t number);
   bool removeShips(int type, uint32_t number);
   int numShips(int type);
   std::map<int, int> getShips() const;
   int totalShips() const;
         
   virtual void packExtraData(Frame * frame);   
   virtual void doOnceATurn(IGObject * obj);   
   virtual int getContainerType();
   
   virtual ObjectData* clone() const;

 private:
   Velocity3dObjectParam *velocity;
   ReferenceObjectParam *player;
   RefQuantityListObjectParam *shipList;
   IntegerObjectParam *damage;
   OrderQueueObjectParam * orders;

};

}

#endif
