/*  Order baseclass
 *
 *  Copyright (C) 2003-2005,2007  Lee Begg and the Thousand Parsec Project
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

#include <stdlib.h>
#include <time.h>

#include "frame.h"
#include "orderparameter.h"
#include "game.h"
#include "ordermanager.h"
#include "orderqueue.h"

#include "order.h"

#include <boost/bind.hpp>

Order::Order(): orderqueueid(0), type(0), name(), description(), turns(0), resources(), parameters()
{
  descmodtime = time(NULL);
}

Order::~Order(){
  for(ParameterList::iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    delete (*itcurr);
  }
}

uint32_t Order::getType() const
{
  return type;
}

void Order::setType(uint32_t ntype){
  type = ntype;
}

std::string Order::getName() const{
  return name;
}

uint32_t Order::getTurns() const{
  return turns;
}

void Order::setTurns(uint32_t nturns){
  turns = nturns;
}

IdMap Order::getResources() const{
  return resources;
}

void Order::addResource(uint32_t resid, uint32_t amount){
  resources[resid] = amount;
}

Order::ParameterList Order::getParameters() const{
  return parameters;
}

void Order::createFrame(Frame * f, int pos)
{
  f->setType(ft02_Order);
  if(f->getVersion() <= fv0_3){
    f->packInt(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  }else{
    f->packInt(orderqueueid);
  }
  f->packInt(pos);
  f->packInt(type);
  f->packInt(turns);
  f->packIdMap(resources);
  std::for_each( parameters.begin(), parameters.end(), boost::bind( &OrderParameter::pack, _1, f ) );
}

void Order::inputFrame(Frame * f, uint32_t playerid)
{
  //ready passed object, position, and type.
  if(!f->isEnoughRemaining(8))
    throw FrameException("Not 8 bytes remaining to read");

  f->unpackInt(); // turns, read only
  int ressize = f->unpackInt(); // size of resource list (should be zero)
  if(!f->isEnoughRemaining(8 * ressize))
    throw FrameException("Not enough bytes remaining");
  for(int i = 0; i < ressize; i++){
    f->unpackInt(); //The resource id
    f->unpackInt(); //The amount of the resource
  }
  for(ParameterList::iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    if (!(*itcurr)->unpack(f)) 
      throw FrameException("Unable to unpack Frame");
  }
}


void Order::describeOrder(Frame * f) const
{
  f->setType(ft02_OrderDesc);
  f->packInt(type);
  f->packString(name);
  f->packString(description);
  f->packInt(parameters.size());
  std::for_each( parameters.begin(), parameters.end(), boost::bind( &OrderParameter::packOrderDescFrame, _1, f ) );
  f->packInt64(descmodtime);
}

uint64_t Order::getDescriptionModTime() const{
  return descmodtime;
}

OrderParameter* Order::addOrderParameter(OrderParameter* op){
  parameters.push_back(op);
  return op;
}

void Order::setOrderQueueId(uint32_t id){
  orderqueueid = id;
}

uint32_t Order::getOrderQueueId() const{
  return orderqueueid;
}
