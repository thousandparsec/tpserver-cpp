#ifndef FLEET_ORDER_H
#define FLEET_ORDER_H
/*  FleetOrder class.  This class is used as a base class for the normal
 *  fleet based orders.  It encorporates many functions and data which
 *  each of these orders need.
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

#include <tpserver/order.h>

class ObjectOrderParameter;

class FleetOrder : public Order
{
 public:
   FleetOrder();
   virtual ~FleetOrder();

   virtual Order* clone() const = 0;

   virtual void createFrame(OutputFrame::Ptr f, int pos);
   virtual void inputFrame(InputFrame * f, uint32_t playerid);
   
   virtual bool doOrder(IGObject::Ptr obj) = 0;

 protected:
   //The star system object this order is targetting.
   ObjectOrderParameter* starSys;
   //A function to return which regions border the fleet
   std::set<uint32_t> getBorderingRegions();
   //Gets the leader of type leaderType in the specified region.
   int getLeaderInRegion(uint32_t region, std::string leaderType);
};


#endif
