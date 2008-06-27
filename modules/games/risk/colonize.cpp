/*  colonize
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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
 
#include <tpserver/objectorderparameter.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/message.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/logging.h>
#include <tpserver/listparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/settings.h>

#include "colonize.h"
#include <boost/format.hpp>
#include "planet.h"

namespace RiskRuleset {

using std::map;
using std::pair;
using std::string;
using std::set;
using boost::format;

Colonize::Colonize() : Order()  {
   name = "Colonize";
   description = "Colonize a planet";

   //ASK: Work with Lee to get colonize order available on unowned planets
   targetPlanet = new ListParameter();
   targetPlanet->setName("Planet");
   targetPlanet->setDescription("The Planet to bid on.");
   targetPlanet->setListOptionsCallback(ListOptionCallback(this,
      &Colonize::generateListOptions));
   addOrderParameter(targetPlanet);

   turns = 1;
}

Colonize::~Colonize() {

}

map<uint32_t, pair<string, uint32_t> > Colonize::generateListOptions() {
   map<uint32_t, pair<string,uint32_t> > options;
   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();

   IGObject* selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet* planet = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());
   assert(planet);
   om->doneWithObject(selectedObj->getID());

   set<uint32_t> allObjs = om->getAllIds();

   uint32_t availibleUnits = planet->getResource("Army").first + planet->getResource("Army").second - 1;

   /* This for loop will iterate over every adjacent planet. 
   This is where the majority of the work occurs, and we populate our list.
   You see here we select an item of the map, in my case (*i)->getID(), and
   for that item we create a pair.
      If its a little hard to read, here is what I am doing:
            options[#] = pair<string,uint32_t>( "title", max# );

   For my pair I set the title as the adjacent planet to move to, and set the
   max to availible units. */
   for(set<uint32_t>::iterator i = allObjs.begin(); i != allObjs.end(); i++) {
      IGObject* currObj = om->getObject((*i));
      Planet* owned = dynamic_cast<Planet*>(currObj->getObjectBehaviour());
      if ( owned != NULL && owned->getOwner() == 0) {   
         options[owned->getID()] = pair<string,uint32_t>(
            owned->getName(), availibleUnits );
      }
   }   
   //CHECK: how to get more than a single digit display

   return options;
}

Order* Colonize::clone() const {
   Colonize *c = new Colonize();
   c->type = this->type;
   return c;
}


bool Colonize::doOrder(IGObject *obj) {
   bool result = true;
   --turns;

   Planet* origin = dynamic_cast<Planet*>(obj->getObjectBehaviour());
   assert(origin);
   Logger::getLogger()->debug("Starting a Colonize::doOrder on %s.",origin->getName().c_str());
   
   //Get the list of objects and the # of units to colonize
   map<uint32_t,uint32_t> list = targetPlanet->getList();
   
   //Collect all of the players bids and restrain them to 1 less than the current units
   map<IGObject*,uint32_t> bids;
   for(map<uint32_t,uint32_t>::iterator i = list.begin(); i != list.end(); ++i) {
      uint32_t planetID = i->first;
      uint32_t numUnits = i->second;
      IGObject* target = Game::getGame()->getObjectManager()->getObject(planetID);     
      
      //Restrain the number of units moved off of origin
      uint32_t maxUnits = origin->getResource("Army").first;
      numUnits = numUnits + bids[target];             //Add current bid on target to requests units (no tricksy bidding!)
      if ( numUnits >= maxUnits && maxUnits > 0) {
         if ( maxUnits > 0)
            numUnits = maxUnits - 1;
         else
            numUnits = 0;
      }
      
      bids[target] = numUnits;
   }
   
   //for each seperate planet bid on run the bid routine
   for(map<IGObject*,uint32_t>::iterator i = bids.begin(); i != bids.end(); ++i) {
      Logger::getLogger()->debug("\tStarting to iterate over all players bids");
      
      Planet* biddedPlanet = dynamic_cast<Planet*>(i->first->getObjectBehaviour());
      assert(biddedPlanet);
      
      //Ensure the object IS a planet and the object is unowned
      //The object MAY be owned if a bid has occured and a winner was chosen
      //then all other bids on that planet will simply be ignored
      if ( biddedPlanet != NULL && biddedPlanet->getOwner() == 0) {
         Logger::getLogger()->debug("\tGetting the top player and bid for planet %s",biddedPlanet->getName().c_str());
        
        //Get pair <owner's planet,bid> of top bidder
         pair<IGObject*,uint32_t> topBidder = getTopPlayerAndBid(i->first);

         //Check if players bid is bigger than top bidder elsewhere - only did this because I wasn't sure
         //BUG: this code would force the reset of bid restriction
         // if ( i->second > topBidder.second ) {
         //    topBidder.second = i->second;
         //    topBidder.first = obj;
         // }
        
         Planet* ownerPlanet = dynamic_cast<Planet*>(topBidder.first->getObjectBehaviour());
         assert(ownerPlanet);
         
         uint32_t player = ownerPlanet->getOwner(); 
         
         biddedPlanet->setOwner(player);
         biddedPlanet->addResource("Army",topBidder.second);
         ownerPlanet->removeResource("Army",topBidder.second);
         
         //Inform colonize winner, and obj owner
      }
      else //Bidded planet is now owned
      {
         Logger::getLogger()->debug("\tNot getting top bidders, planet is already owned");
         //inform player bid failed, obj is now owned by _person_, won bid with _#units_
      }
   }
   
   return result;
}

//This function gets the top bid for the given object
pair<IGObject*,uint32_t> Colonize::getTopPlayerAndBid(IGObject* obj) {
   pair<IGObject*,uint32_t> result;
   result.first = NULL;
   result.second = 0;
   
   Planet* origin = dynamic_cast<Planet*>(obj->getObjectBehaviour());
   assert(origin);

   Logger::getLogger()->debug("\tCollecting all bids on object %s",origin->getName().c_str());

   Game* game = Game::getGame();
   OrderManager* ordM = game->getOrderManager();
   ObjectManager* objM = game->getObjectManager();
   
   //Construct the map to be used, the identifier is the planet, the value is the bid
   map<IGObject*,uint32_t> bids;
      
   //Get all objects from object manager
   set<uint32_t> objectsIds = objM->getAllIds();
   
   //Iterate over every object
   for(set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i)
   {
      //Get current object
      IGObject * currObj = objM->getObject(*i);
      
      //Print out current planet
      Planet* bidder = dynamic_cast<Planet*>(currObj->getObjectBehaviour());
      if (bidder != NULL) {
         Logger::getLogger()->debug("\t\tLooking at orders on object %s",bidder->getName().c_str());
      }
      
      //Get order queue from object
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(currObj->getParameterByType(obpT_Order_Queue));
      OrderQueue* oq;

      //Validate that the oq exists
      if(oqop != NULL && (oq = ordM->getOrderQueue(oqop->getQueueId())) != NULL)
      {
         //Iterate over all orders
         for (uint32_t j = 0; j < oq->getNumberOrders(); j++)
         {
            OwnedObject *orderedObj = dynamic_cast<OwnedObject*>(currObj->getObjectBehaviour());
            Order* order = NULL;
            if ( orderedObj != NULL ) {
               order = oq->getOrder(j, orderedObj->getOwner());
            }
            
            Logger::getLogger()->debug("\t\tThere exists a %s order on %s", order->getName().c_str(), bidder->getName().c_str());
            //if order is a colonize order
            if( order != NULL && order->getName() == "Colonize")
            {
               Colonize* colonize = dynamic_cast<Colonize*>(order);
               assert(colonize);

               //Get the list of planetIDs and the # of units to move
               map<uint32_t,uint32_t> list = colonize->getTargetList()->getList();
               
               //Iterate over all suborders
               for(map<uint32_t,uint32_t>::iterator i = list.begin(); i != list.end(); ++i) {
                  uint32_t planetID = i->first;
                  uint32_t numUnits = i->second;
                  
                  format debug("\t\t\tEncountered suborder to Colonize %1% with %2% units");
                  debug % planetID; debug % numUnits; 
                  Logger::getLogger()->debug(debug.str().c_str());
                  
                  IGObject* target = Game::getGame()->getObjectManager()->getObject(planetID);
                  if ( target == obj ) {
                     bids[currObj] += numUnits;                     
                  }
               }
            }
            else if ( order->getName() != "Colonize")
            {
               j = oq->getNumberOrders() + 1;   //force the loop to exit, we have "left" the frontal Colonize orders
            }
         }
         currObj->touchModTime();
      }
      objM->doneWithObject(currObj->getID());
   }
   

   //Iterate over all bids and restrict them to the maximum armies availible on that planet
   for(map<IGObject*,uint32_t>::iterator i = bids.begin(); i != bids.end(); ++i) {
      Logger::getLogger()->debug("Iterating over all bids to pick the highest legal bid.");
      //Restrict players bid to 1 less than their current reinforcements
      Planet* planet = dynamic_cast<Planet*>(i->first->getObjectBehaviour());
      assert(planet);
      
      uint32_t numUnits = i->second;
      uint32_t maxUnits = planet->getResource("Army").first;
      if ( numUnits >= maxUnits && maxUnits > 0) {
         if ( maxUnits > 0)
            numUnits = maxUnits - 1;
         else
            numUnits = 0;
      }
      i->second = numUnits;
      
      if ( numUnits > result.second ) {
         result.second = i->second;
         result.first = i->first;
      }
   }
   //LATER: Aggregate all bids from multiple objects to their owners

   sendPlayerMessages(obj, bids,result);
   
   format debugMsg("Found highest bidder to be from planet #%1% with %2% units");
   debugMsg % result.first->getName(); debugMsg % result.second;
   Logger::getLogger()->debug(debugMsg.str().c_str());
   
   return result;
}

//TODO: Check that "this" move order owner gets a message
void Colonize::sendPlayerMessages(IGObject* obj, map<IGObject*,uint32_t> bids, 
      pair<IGObject*,uint32_t> winner) {
         
   PlayerManager* pm = Game::getGame()->getPlayerManager();
   Planet* target = dynamic_cast<Planet*>(obj->getObjectBehaviour());
   assert(target);
   
   //Message subjects
   string loserSubject = "Colonize Bid for " + target->getName() + " Rejected";
   string winnerSubject = "Colonize Bid for " + target->getName() + " Accepted";
   
   for(map<IGObject*,uint32_t>::iterator i = bids.begin(); i != bids.end(); i++ ) {
      Planet* ownerPlanet = dynamic_cast<Planet*>(i->first->getObjectBehaviour());
      assert(ownerPlanet);
      Player* player = pm->getPlayer(ownerPlanet->getOwner());
      assert(player);
      
      //Populate message's subject and body
      string subject;
      format body ("Colonize bid via %1% to colonize %2% with %3% units was %4%."); 
      body % ownerPlanet->getName(); body % target->getName(); body % i->second;
      if ( i->first == winner.first ) { //If this is the winning bid
         subject = winnerSubject;
         body % "Accepted";
      }
      else {                              //The bid did not win
         subject = loserSubject;
         body % "Rejected";
      }
      
      Message* msg = new Message();
      msg->setSubject(subject);
      msg->setBody(body.str());
      player->postToBoard(msg);
   }
   
}

ListParameter* Colonize::getTargetList() {
   return targetPlanet;
}

}
