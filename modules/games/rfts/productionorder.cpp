/*  production order
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

#include <cassert>

#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/frame.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/ordermanager.h>
#include <tpserver/listparameter.h>
#include <tpserver/objectmanager.h>

#include "rfts.h"
#include "productioninfo.h"
#include "planet.h"

#include "productionorder.h"

namespace RFTS_ {

using std::string;
using std::map;
using std::pair;

ProductionOrder::ProductionOrder() {
   name = "Produce";
   description = "Order the production of planetary stats";
   
   productionList = new ListParameter();
   productionList->setName("Planetary stats");
   productionList->setDescription("The production orders");
   productionList->setListOptionsCallback(ListOptionCallback(this,
            &ProductionOrder::generateListOptions));
   
   addOrderParameter(productionList);
}

ProductionOrder::~ProductionOrder() {

}

map<uint32_t, pair<string, uint32_t> >ProductionOrder::generateListOptions() {
   map<uint32_t, pair< string, uint32_t> > options;

   Game* game = Game::getGame();
   
   IGObject *selectedObj = game->getObjectManager()->getObject(
            game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet *planet = dynamic_cast<Planet*>(selectedObj->getObjectData());

   assert(planet);
   
   game->getObjectManager()->doneWithObject(selectedObj->getID());

   setOption(options, "Industry", planet);
   setOption(options, "Social Environment", planet);
   setOption(options, "Planetary Environment", planet);
   setOption(options, "Population Maintenance", planet);
   setOption(options, "Colonist", planet);
   
   return options;
}

void ProductionOrder::setOption(map<uint32_t, pair<string, uint32_t> >& options, 
               const string& resTypeName, Planet* planet) {
   // resTypeName->ID : resTypeName, current RP / costOfRes (cap at MaxRes)
  options[ Game::getGame()->getResourceManager()->
           getResourceDescription(resTypeName)->getResourceType() ] =
            pair<string, uint32_t>(resTypeName,
            planet->getCurrentRP() / Rfts::getProductionInfo().getResourceCost(resTypeName));
}

void ProductionOrder::createFrame(Frame *f, int pos) {
   Order::createFrame(f, pos);
}

Result ProductionOrder::inputFrame(Frame *f, unsigned int playerid) {
   return Order::inputFrame(f, playerid);
}

bool ProductionOrder::doOrder(IGObject *obj) {

   Game *game = Game::getGame();
   ResourceManager *resMan = game->getResourceManager();
   Planet *planet = dynamic_cast<Planet*>(obj->getObjectData());
   assert(planet);

   map<uint32_t, uint32_t> list = productionList->getList();
   
   for(map<uint32_t, uint32_t> ::iterator i = list.begin(); i != list.end(); ++i)
   {
      // remove the RPs for this each of this resource
      string resTypeName = resMan->getResourceDescription(i->first)->getNameSingular();
      uint32_t resCost = Rfts::getProductionInfo().getResourceCost(resTypeName);

      planet->removeResource("Resource Point", i->second * resCost);
      planet->addResource(i->first, i->second);
   }

   return true;
}

Order* ProductionOrder::clone() const {
   ProductionOrder *c = new ProductionOrder();
   c->type = this->type;
   return c;
}

}
