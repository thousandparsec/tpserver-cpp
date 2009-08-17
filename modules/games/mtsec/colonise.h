#ifndef COLONISE_H
#define COLONISE_H
/*  Colonise Order
 *
 *  Copyright (C) 2004-2005,2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/result.h>
#include <tpserver/order.h>

class Move;
class ObjectOrderParameter;

namespace MTSecRuleset {

class Colonise : public Order {
 public:
  Colonise();
  virtual ~Colonise();
  
  bool doOrder(IGObject * ob);

  Order* clone() const;


};

}
#endif
