#ifndef ORDER_H
#define ORDER_H
/*  Order base class
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

#include <tpserver/common.h>
#include <tpserver/object.h>

class OrderParameter;

class Order {

  public:
    typedef std::list<OrderParameter*> ParameterList;
    Order();
    virtual ~Order();

    uint32_t getType() const;
    void setType(uint32_t ntype);
    std::string getName() const;
    uint32_t getTurns() const;
    void setTurns(uint32_t nturns);
    IdMap getResources() const;
    void addResource(uint32_t resid, uint32_t amount);
    ParameterList getParameters() const;
    uint64_t getDescriptionModTime() const;

    virtual void createFrame(OutputFrame::Ptr f, int pos);
    // throws FrameException
    virtual void inputFrame(InputFrame::Ptr f, uint32_t playerid);

    virtual bool doOrder(IGObject::Ptr ob) = 0;

    void describeOrder(OutputFrame::Ptr f) const;
    virtual Order *clone() const = 0;

    //For OrderQueue only
    void setOrderQueueId(uint32_t id);
    uint32_t getOrderQueueId() const;

  protected:
    OrderParameter* addOrderParameter(OrderParameter* op);
    uint32_t orderqueueid;
    uint32_t type;
    uint64_t descmodtime;
    std::string name;
    std::string description;
    uint32_t turns;
    IdMap resources;

  private:
    ParameterList parameters;

};

#endif
