#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H
/*  OrderManager class
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

#include <tpserver/common.h>
#include <tpserver/orderqueue.h>
#include <tpserver/inputframe.h>
#include <tpserver/outputframe.h>

class Order;

class OrderManager{
  public:
    typedef boost::shared_ptr<OrderQueue> Ptr;

    OrderManager();
    ~OrderManager();

    bool checkOrderType(uint32_t type);
    void describeOrder(uint32_t ordertype, OutputFrame::Ptr f);
    void addOrderType(Order* prototype);
    uint32_t getOrderTypeByName(const std::string &name);

    Order* createOrder(uint32_t ot);

    uint32_t addOrderQueue( uint32_t objectid, uint32_t ownerid );
    OrderQueue::Ptr getOrderQueue(uint32_t oqid);
    std::map<uint32_t, OrderQueue::Ptr> getOrderQueues();
    void updateOrderQueue(uint32_t oqid);
    bool removeOrderQueue(uint32_t oqid);

    // TODO: refactor
    uint32_t getSeqKey() const { return seqkey; }
    IdModList getModList( uint64_t fromtime ) const;
    void init();

  private:
    typedef std::map<uint32_t, Order*> PrototypeStore;
    typedef std::map<uint32_t, OrderQueue::Ptr> OrderQueueStore;

    PrototypeStore  prototype_store;
    OrderQueueStore orderqueue_store;
    NameMap typename_map;

    uint32_t prototype_next;
    uint32_t orderqueue_next;
    uint32_t seqkey;
};

#endif
