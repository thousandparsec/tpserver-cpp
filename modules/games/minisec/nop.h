#ifndef NOP_H
#define NOP_H
/*  Nop order
 *
 *  Copyright (C) 2004,2007  Lee Begg and the Thousand Parsec Project
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

//class Order;
#include <tpserver/order.h>

class TimeParameter;

class Nop:public Order {
  public:
    Nop();
    virtual ~Nop();

    void createFrame(OutputFrame::Ptr f, int pos);
    void inputFrame(InputFrame * f, uint32_t playerid);

    bool doOrder(IGObject::Ptr ob);

    Order* clone() const;

  private:
    TimeParameter* timeparam;

};

#endif
