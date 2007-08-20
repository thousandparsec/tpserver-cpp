#ifndef colonise_H
#define colonise_H
/*  colonise class
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

#include <tpserver/order.h>

class ObjectOrderParameter;
class IGObject;

namespace RFTS_ {

class Colonise : public ::Order {

 public:
   Colonise();
   virtual ~Colonise();

   virtual Order* clone() const;

   virtual bool doOrder(IGObject* obj);

 private:
   ObjectOrderParameter *planet;

   static const unsigned VICTORY_POINTS_ATTACK = 25;
   static const unsigned VICTORY_POINTS_COLONISE = 125;
};
}

#endif
