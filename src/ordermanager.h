#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H
/*  OrderManager class
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

#include <map>

class Order;
class Frame;

class OrderManager{
 public:
  OrderManager();
  ~OrderManager();

  
  bool checkOrderType(int type);
  void describeOrder(int ordertype, Frame * f);
  Order* createOrder(int ot);

  void addOrderType(Order* prototype);

 private:
  std::map<int, Order*> prototypeStore;
  int nextType;


};

#endif
