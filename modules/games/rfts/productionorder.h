#ifndef PRODUCTION_ORDER_H
#define PRODUCTION_ORDER_H
/*  production order class
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

#include <sstream>

#include <tpserver/order.h>

class IGObject;
class Frame;
class ListParameter;

namespace RFTS_ {

class Planet;

class ProductionOrder : public ::Order {

 public:
   ProductionOrder();
   virtual ~ProductionOrder();

   virtual void createFrame(Frame * f, int pos);
   virtual void inputFrame(Frame * f, uint32_t playerid);

   virtual bool doOrder(IGObject::Ptr obj);

   virtual Order* clone() const;
 private:

   std::map<uint32_t, std::pair<std::string, uint32_t> > generateListOptions();
   void setOption(std::map<uint32_t, std::pair<std::string, uint32_t> >& options,
               const std::string& resTypeName, Planet* planet);

   ListParameter *productionList;

};

}

#endif
