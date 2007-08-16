#ifndef bombard_H
#define bombard_H
/*  bombard class
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

class IGObject;
class ObjectOrderParameter;
class Frame;

namespace RFTS_{

class Bombard : public ::Order {

 public:
   Bombard();
   virtual ~Bombard();

   virtual Order* clone() const;

   virtual Result inputFrame(Frame * f, unsigned int playerid);

   virtual bool doOrder(IGObject *obj);

 private:
   ObjectOrderParameter *planet;

   static const unsigned POPULATION_DMG = 8;
   static const unsigned INDUSTRY_DMG = 1;
   static const unsigned SOCIAL_DMG = 5;
   static const unsigned PLANETARY_DMG = 4;
   
};

}

#endif
