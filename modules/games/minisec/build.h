#ifndef BUILD_H
#define BUILD_H
/*  BuildFleet order
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/orderparameters.h>

class Build : public Order{
  public:
    Build();
    virtual ~Build();

    void createFrame(Frame *f, int pos);
    void inputFrame(InputFrame *f, uint32_t playerid);

    bool doOrder(IGObject::Ptr ob);

    Order* clone() const;

  private:
    ListParameter::Options generateListOptions();
    ListParameter * fleetlist;
    StringParameter * fleetname;
};

#endif
