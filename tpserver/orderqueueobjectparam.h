#ifndef ORDERQUEUEOBJECTPARAM_H
#define ORDERQUEUEOBJECTPARAM_H
/*  OrderQueueObjectParam class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>
#include <set>

#include <tpserver/objectparameter.h>

class OrderQueueObjectParam : public ObjectParameter{

public:
  OrderQueueObjectParam();
  virtual ~OrderQueueObjectParam();

  virtual void packObjectFrame(Frame * f, uint32_t playerid);
  virtual void packObjectDescFrame(Frame * f);
  virtual bool unpackModifyObjectFrame(Frame * f, unsigned int playerid);

  virtual void signalRemoval();
  
  virtual ObjectParameter *clone() const;
  
  uint32_t getQueueId() const;
  void setQueueId(uint32_t ob);
  
  uint32_t getNumOrders() const;

  
  std::set<uint32_t> getAllowedOrders() const;
  void setAllowedOrders(std::set<uint32_t> ao);

protected:
  uint32_t queueid;
  uint32_t numorders;
  
  std::set<uint32_t> ordertypes;

};

#endif
