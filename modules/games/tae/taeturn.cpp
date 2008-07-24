/*  TaeTurn object
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/game.h>
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/playermanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/ordermanager.h>
#include <tpserver/message.h>
#include <tpserver/logging.h>

#include "planet.h"
#include "starsystem.h"
#include "fleet.h"
#include "fleetbuilder.h"

#include "taeturn.h"

using std::stringstream;
using std::string;
using std::map;
using std::set;


TaeTurn::TaeTurn(FleetBuilder* fb) : TurnProcess(), containerids(){
    fleetBuilder = fb;
    playerTurn = 0;
    combat = false;
    isInternal = false;
}

TaeTurn::~TaeTurn(){

}

void TaeTurn::doTurn(){
    if(combat) {
        doCombatTurn();
        return;
    }

    std::set<uint32_t>::iterator itcurr;

    Game* game = Game::getGame();
    OrderManager* ordermanager = game->getOrderManager();
    ObjectManager* objectmanager = game->getObjectManager();
    PlayerManager* playermanager = game->getPlayerManager();

    //build map for storing orders
    std::map<uint32_t, std::list<IGObject*> > playerOrders;

    //do orders 
    containerids.clear();

    std::set<uint32_t> objects = objectmanager->getAllIds();
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->getType() == planettype || ob->getType() == fleettype){
            OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
            if(oqop != NULL){
                OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
                if(orderqueue != NULL){
                    Order * currOrder = orderqueue->getFirstOrder();
                    if(currOrder != NULL){
                        uint32_t owner = ((OwnedObject*)(ob->getObjectBehaviour()))->getOwner();
                        std::list<IGObject*>::iterator i = playerOrders[owner].end();
                        playerOrders[owner].insert(i, ob);
                    }
                }
            }
        }
    }

    //Do orders for players in the correct order
    std::set<uint32_t> players = playermanager->getAllIds();
    itcurr = players.begin();
    for(int i = 0; i < playerTurn; i++) {
        itcurr++;
    }
    for(int it = 0; it < players.size(); it++) {
        if(itcurr == players.end()) {
            itcurr = players.begin();
        }
        if(playerOrders[*itcurr].size() > 0) {
            for(std::list<IGObject*>::iterator i = playerOrders[*itcurr].begin(); i != playerOrders[*itcurr].end(); i++) {
                OrderQueue* orderqueue = ordermanager->getOrderQueue(((OrderQueueObjectParam*)((*i)->getParameterByType(obpT_Order_Queue)))->getQueueId());
                Order* currOrder = orderqueue->getFirstOrder();
                if(currOrder!= NULL) {
                    if(currOrder->doOrder(*i)) {
                        orderqueue->removeFirstOrder();
                    } else {
                        orderqueue->updateFirstOrder();
                    }
                }
                if((*i)->getContainerType() >= 1){
                    containerids.insert((*i)->getID());
                }
                objectmanager->doneWithObject((*i)->getID());
            }
        }
        itcurr++;
    }
    
    awardArtifacts();

    //Initialize combat if the next turn is a combat turn
    if(combat) {
        initCombat();
    }

    //Update which player's turn it is
    playerTurn = (playerTurn + 1) % playermanager->getNumPlayers();

    objectmanager->clearRemovedObjects();


    // to once a turn (right at the end)
    objects = objectmanager->getAllIds();
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->isAlive()){
            ob->getObjectBehaviour()->doOnceATurn();
        }
        objectmanager->doneWithObject(ob->getID());
    }

    // find the objects that are visible to each player
    std::set<uint32_t> vis = objectmanager->getAllIds();
    for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
        Player* player = playermanager->getPlayer(*itplayer);
        PlayerView* playerview = player->getPlayerView();

        for(std::set<uint32_t>::iterator itob = vis.begin(); itob != vis.end(); ++itob){
            ObjectView* obv = playerview->getObjectView(*itob);
            if(obv == NULL){
                if(objectmanager->getObject(*itob)->isAlive()){
                    obv = new ObjectView();
                    obv->setObjectId(*itob);
                    obv->setCompletelyVisible(true);
                    playerview->addVisibleObject(obv);
                }
                objectmanager->doneWithObject(*itob);
            }else{
                IGObject* ro = objectmanager->getObject(*itob);
                uint64_t obmt = ro->getModTime();
                objectmanager->doneWithObject(*itob);
                if(obmt > obv->getModTime()){
                    obv->setModTime(obmt);
                    playerview->updateObjectView(*itob);
                }
            }
        }

        // remove dead objects
        std::set<uint32_t> goneobjects;
        std::set<uint32_t> knownobjects = playerview->getVisibleObjects();
        set_difference(knownobjects.begin(), knownobjects.end(), vis.begin(), vis.end(), inserter(goneobjects, goneobjects.begin()));

        for(std::set<uint32_t>::iterator itob = goneobjects.begin(); itob != goneobjects.end(); ++itob){
            ObjectView* obv = playerview->getObjectView(*itob);
            if(!obv->isGone()){
                obv->setGone(true);
                playerview->updateObjectView(*itob);
            }
        }

        //Replace colonist fleets
        int fleets = 0;
        IGObject* homePlanet;
        for(std::set<uint32_t>::iterator itob = objects.begin(); itob != objects.end(); ++itob){    
            IGObject * ob = objectmanager->getObject(*itob);
            if(ob->getName().compare("Colonist Fleet") == 0) {
                uint32_t owner = ((OwnedObject*)(ob->getObjectBehaviour()))->getOwner();                if(owner == *itplayer) {
                    fleets++;
                }
            }
            if(ob->getName().compare(string(player->getName() + "'s Home Planet")) == 0) {
                homePlanet = ob;
            }
        }

        for(int i = fleets; i < 6; i++) {
            IGObject* fleet = fleetBuilder->createFleet(FleetBuilder::PASSENGER_FLEET, FleetBuilder::RANDOM_SHIP, player, homePlanet, "Colonist Fleet");
            game->getObjectManager()->addObject(fleet);
            ObjectView* obv = new ObjectView();
            obv->setObjectId(fleet->getID());
            obv->setCompletelyVisible(true);
            player->getPlayerView()->addVisibleObject(obv);
        }

        //Send end of turn message to each player
        Message * msg = new Message();
        msg->setSubject("Turn complete");
        stringstream out;
        out << "Your Current Score: \n";
        out << "Money: " << player->getScore(1) << "\n";
        out << "Technology: " << player->getScore(2) << "\n";
        out << "People: " << player->getScore(3) << "\n";
        out << "Raw Materials: " << player->getScore(4) << "\n";
        out << "Alien Artifacts: " << player->getScore(5);
        msg->setBody(out.str());
        player->postToBoard(msg);

        //Alert players to the turn order for next round
        msg = new Message();
        msg->setSubject("Turn Order");
        string body = "The order for the next turn is: ";
        itcurr = players.begin();
        for(int i = 0; i < playerTurn; i++) {
            itcurr++;
        }
        for(int it = 0; it < players.size(); it++) {
            if(itcurr == players.end()) {
                itcurr = players.begin();
            }
            body += playermanager->getPlayer(*itcurr)->getName();
            body += " ";
            itcurr++;
        }
        msg->setBody(body);
        player->postToBoard(msg);
    }

    playermanager->updateAll();

}

void TaeTurn::initCombat() {
    std::set<uint32_t>::iterator itcurr;

    Game* game = Game::getGame();
    ObjectManager* objectmanager = game->getObjectManager();
    PlayerManager* playermanager = game->getPlayerManager();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    DesignStore* ds = game->getDesignStore();

    set<uint32_t> owners;
    string shipType;
    for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
        Fleet* leader = (Fleet*) (objectmanager->getObject(i->first))->getObjectBehaviour();
        if(shipType.empty()) {
            if(isInternal) {
                shipType = "ScientistShip";
            } else {
                uint32_t ship = leader->getShips().begin()->first;
                shipType = ds->getDesign(ship)->getName();
            }
        }

        owners.insert(leader->getOwner());
    }
            
    std::set<ObjectView*> views;
    std::set<uint32_t> objects = objectmanager->getAllIds();
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->getType() == obtm->getObjectTypeByName("Fleet")) {
            Fleet* f = (Fleet*) ob->getObjectBehaviour();
            f->toggleCombat();
            //check to see if this fleet is a combatant
            if(owners.count(f->getOwner()) > 0) {
                uint32_t ship = f->getShips().begin()->first;
                string name = ds->getDesign(ship)->getName();
                if(name.compare(shipType) == 0) {
                    f->setCombatant(true);
                }
            }

            //Set visibility
            ObjectView* obv = new ObjectView();
            obv->setObjectId(ob->getID());
            obv->setCompletelyVisible(true);
            views.insert(obv);
        }
    }

    Message * msg = new Message();
    msg->setSubject("COMBAT!");
    msg->setBody(string("The next turn is a combat turn!"));
    
    std::set<uint32_t> players = playermanager->getAllIds();
    for(itcurr = players.begin(); itcurr != players.end(); ++itcurr) {
        Player* player = playermanager->getPlayer(*itcurr);        
        player->postToBoard(msg);
        for(std::set<ObjectView*>::iterator i = views.begin(); i != views.end(); ++i) {
            player->getPlayerView()->addVisibleObject(*i);
        }
    }
}

void TaeTurn::doCombatTurn() {
    // Do orders
    std::set<uint32_t>::iterator itcurr;

    Game* game = Game::getGame();
    OrderManager* ordermanager = game->getOrderManager();
    ObjectManager* objectmanager = game->getObjectManager();
    PlayerManager* playermanager = game->getPlayerManager();

    containerids.clear();

    std::set<uint32_t> objects = objectmanager->getAllIds();
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->getType() == planettype || ob->getType() == fleettype){
            OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
            if(oqop != NULL){
                OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
                if(orderqueue != NULL){
                    Order * currOrder = orderqueue->getFirstOrder();
                    if(currOrder != NULL){
                        if(currOrder->doOrder(ob)) {
                            orderqueue->removeFirstOrder();
                        } else {
                            orderqueue->updateFirstOrder();
                        }
                    }
                    if(ob->getContainerType() >= 1){
                        containerids.insert(ob->getID());
                    }
                    objectmanager->doneWithObject((ob)->getID());
                }
            }
        }
    }

    //TODO: Remove loosing combatants
    
    //TODO: Check for combat

    //TODO: Award points

    std::set<uint32_t> players = playermanager->getAllIds();
    std::set<uint32_t> vis = objectmanager->getAllIds();

    objectmanager->clearRemovedObjects();
    objects = objectmanager->getAllIds();
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->isAlive()){
            ob->getObjectBehaviour()->doOnceATurn();
        }
        objectmanager->doneWithObject(ob->getID());
    }

    for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
        Player* player = playermanager->getPlayer(*itplayer);
        PlayerView* playerview = player->getPlayerView();

        for(std::set<uint32_t>::iterator itob = vis.begin(); itob != vis.end(); ++itob){
            ObjectView* obv = playerview->getObjectView(*itob);
            if(obv == NULL){
                if(objectmanager->getObject(*itob)->isAlive()){
                    obv = new ObjectView();
                    obv->setObjectId(*itob);
                    obv->setCompletelyVisible(true);
                    playerview->addVisibleObject(obv);
                }
                objectmanager->doneWithObject(*itob);
            }else{
                IGObject* ro = objectmanager->getObject(*itob);
                uint64_t obmt = ro->getModTime();
                objectmanager->doneWithObject(*itob);
                if(obmt > obv->getModTime()){
                    obv->setModTime(obmt);
                    playerview->updateObjectView(*itob);
                }
            }
        }

        // remove dead objects
        std::set<uint32_t> goneobjects;
        std::set<uint32_t> knownobjects = playerview->getVisibleObjects();
        set_difference(knownobjects.begin(), knownobjects.end(), vis.begin(), vis.end(), inserter(goneobjects, goneobjects.begin()));

        for(std::set<uint32_t>::iterator itob = goneobjects.begin(); itob != goneobjects.end(); ++itob){
            ObjectView* obv = playerview->getObjectView(*itob);
            if(!obv->isGone()){
                obv->setGone(true);
                playerview->updateObjectView(*itob);
            }
        }

        //Replace colonist fleets
        int fleets = 0;
        IGObject* homePlanet;
        for(std::set<uint32_t>::iterator itob = objects.begin(); itob != objects.end(); ++itob){    
            IGObject * ob = objectmanager->getObject(*itob);
            if(ob->getName().compare("Colonist Fleet") == 0) {
                uint32_t owner = ((OwnedObject*)(ob->getObjectBehaviour()))->getOwner();                if(owner == *itplayer) {
                    fleets++;
                }
            }
            if(ob->getName().compare(string(player->getName() + "'s Home Planet")) == 0) {
                homePlanet = ob;
            }
        }

        for(int i = fleets; i < 6; i++) {
            IGObject* fleet = fleetBuilder->createFleet(FleetBuilder::PASSENGER_FLEET, FleetBuilder::RANDOM_SHIP, player, homePlanet, "Colonist Fleet");
            game->getObjectManager()->addObject(fleet);
            ObjectView* obv = new ObjectView();
            obv->setObjectId(fleet->getID());
            obv->setCompletelyVisible(true);
            player->getPlayerView()->addVisibleObject(obv);
        }

        //TODO: Send end of turn message to each player

    }


    playermanager->updateAll();

}

void TaeTurn::awardArtifacts() {
    Game* game = Game::getGame();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    ObjectManager* objectmanager = game->getObjectManager();
    
    std::set<uint32_t> artifacts;
    std::set<uint32_t> regions;
    std::set<uint32_t> objects = objectmanager->getAllIds();
    std::set<uint32_t>::iterator itcurr;

    //Find any regions with 2 or more alien artifacts
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = objectmanager->getObject(*itcurr);
        if(ob->getType() == obtm->getObjectTypeByName("Planet")) {
            Planet* p = (Planet*) ob->getObjectBehaviour();
            if(p->getResource(3) > 0) {
                StarSystem* sys = (StarSystem*)(objectmanager->getObject(ob->getParent())->getObjectBehaviour());
                if(sys->getRegion() != 0) {
                    if(regions.count(sys->getRegion()) > 0) {
                        artifacts.insert(*itcurr);
                    } else {
                        regions.insert(sys->getRegion());
                    }
                }
            }
        }
    }
    
    if(!artifacts.empty()) {
        uint32_t type;
        DesignStore* ds = game->getDesignStore();
        PlayerManager* pm = game->getPlayerManager();
        std::set<unsigned int> designs = ds->getDesignIds();
        //get leader ID
        for(itcurr = designs.begin(); itcurr != designs.end(); ++itcurr) {
            if(ds->getDesign(*itcurr)->getName().compare("MerchantLeaderShip")) {
                type = *itcurr;
            }
        }
        //Search the objects for a merchant leader
        for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = objectmanager->getObject(*itcurr);
            if(ob->getType() == obtm->getObjectTypeByName("Fleet")) {
                Fleet* f = (Fleet*) (ob->getObjectBehaviour());
                if(f->getShips().count(type) > 0) {
                    IGObject* parent = objectmanager->getObject(ob->getParent());
                    if(parent->getType() == obtm->getObjectTypeByName("Star System")) {
                        StarSystem* parentData = (StarSystem*) (parent->getObjectBehaviour());
                        //See if this leader is in a region with
                        //2 or more alien artifacts
                        for(std::set<uint32_t>::iterator i = artifacts.begin(); i != artifacts.end(); ++i) {
                            IGObject* obj = objectmanager->getObject(*i);
                            Planet* p = (Planet*) obj->getObjectBehaviour();
                            StarSystem* sys = (StarSystem*)(objectmanager->getObject(obj->getParent())->getObjectBehaviour());
                            if(sys->getRegion() == parentData->getRegion()) {
                                //+1 to leader's owner's artifact score
                                Player* owner = pm->getPlayer(f->getOwner());
                                owner->setScore(5, owner->getScore(5) + 1);
                                p->removeResource(3, 1);
                                artifacts.erase(*i);
                            }
                        }
                    }
                }
            } 
        }
    }
}

void TaeTurn::queueCombatTurn(bool internal, std::map<uint32_t, uint32_t> com) {
    combat = true;
    isInternal = internal;
    combatants = com;
}

void TaeTurn::addReinforcement(uint32_t player) {
    //TODO: +1 to player's combat strength for this turn
    Logger::getLogger()->debug("Add Reinforcement");
}

void TaeTurn::setPlanetType(uint32_t pt){
    planettype = pt;
}

void TaeTurn::setFleetType(uint32_t ft){
    fleettype = ft;
}

std::set<uint32_t> TaeTurn::getContainerIds() const{
    return containerids;
}
