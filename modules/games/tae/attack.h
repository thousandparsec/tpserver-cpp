#ifndef ATTACK_H
#define ATTACK_H
/*  Attack class
 *
 *  This class defines the attack FleetOrder used by bomber fleets.
 *  The attack order targets a system and destroys it, removing any
 *  colony or leader and making it unusable for the rest of the game.
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

#include "fleetorder.h"

class ObjectOrderParameter;

class Attack : public FleetOrder
{
 public:
   Attack();
   virtual ~Attack();

   virtual Order* clone() const;

   virtual void createFrame(OutputFrame * f, int pos);
   
   virtual bool doOrder(IGObject::Ptr obj);
};


#endif
