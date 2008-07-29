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
    set<uint32_t> regions;
    string shipType;
    for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
        IGObject* ob = objectmanager->getObject(i->first);
        Fleet* leader = (Fleet*) (ob)->getObjectBehaviour();
        if(shipType.empty()) {
            if(isInternal) {
                shipType = "ScientistShip";
            } else {
                uint32_t ship = leader->getShips().begin()->first;
                shipType = ds->getDesign(ship)->getName();
                size_t pos = shipType.find("Leader");
                if(pos != shipType.npos) {
                    shipType.erase(pos, 6);
                }
            }
        }
        owners.insert(leader->getOwner());

        if(regions.count(i->second) <= 0) {
            regions.insert(i->second);
        }

        //Set initial internal combat strength
        if(isInternal) {
            IGObject *starSys = objectmanager->getObject(ob->getParent());
            StarSystem* starSysData = (StarSystem*)(starSys->getObjectBehaviour());
            Vector3d pos = starSysData->getPosition();
            //east-west neighbors
            for(int i = -1; i < 2; i+=2) {
                set<uint32_t> ids = objectmanager->getObjectsByPos(pos+Vector3d(80000*i,0,0), 1);
                for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
                    IGObject *tempObj = objectmanager->getObject(*j);
                    if(tempObj->getType() == obtm->getObjectTypeByName("Planet")) {
                        Planet* p = (Planet*)(tempObj->getObjectBehaviour());
                        if(p->getResource(5) > 0) {
                            addReinforcement(leader->getOwner());
                        }
                    }
                }
            }
            //north-south neighbors
            for(int i = -1; i < 2; i+=2) {
                set<uint32_t> ids = objectmanager->getObjectsByPos(pos+Vector3d(0,80000*i,0), 1);
                for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
                    IGObject *tempObj = objectmanager->getObject(*j);
                    if(tempObj->getType() == obtm->getObjectTypeByName("Planet")) {
                        Planet* p = (Planet*)(tempObj->getObjectBehaviour());
                        if(p->getResource(5) > 0) {
                            addReinforcement(leader->getOwner());
                        }
                    }
                }
            }
        }
    }

    int resourceType;
    if(shipType.compare("MerchantShip") == 0) {
        resourceType = 4;
    } else if(shipType.compare("ScientistShip") == 0) {
        resourceType = 5;
    } else if(shipType.compare("SettlerShip") == 0) {
        resourceType = 6;
    } else {
        resourceType = 7;
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
        //Set initial external combat strength
        else if(ob->getType() == obtm->getObjectTypeByName("Planet") && !isInternal) {
            Planet* p = (Planet*) ob->getObjectBehaviour();
            StarSystem* sys = (StarSystem*) objectmanager->getObject(ob->getParent())->getObjectBehaviour();
            if(regions.count(sys->getRegion()) > 0) {
                if(p->getResource(resourceType) > 0) {
                    for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
                        if(i->second == sys->getRegion()) {
                            Fleet* leader = (Fleet*) (objectmanager->getObject(i->first))->getObjectBehaviour();
                            addReinforcement(leader->getOwner());
                        }
                    }
                }
            }
        }
    }

    Message * msg = new Message();
    msg->setSubject("COMBAT!");
    stringstream out;
    out << "The next turn is an ";
    if(isInternal) {
        out << "INTERNAL ";
    } else {
        out << "EXTERNAL ";
    }
    out << "combat turn! Combatants are:  ";
    out << playermanager->getPlayer(*owners.begin())->getName();
    out << " with an initial strength of ";
    out << strength[*owners.begin()] << " and ";
    out << playermanager->getPlayer(*owners.end())->getName();
    out << " with an initial strength of ";
    out << strength[*owners.end()];
    msg->setBody(out.str());
    
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
    ObjectTypeManager* obtm = game->getObjectTypeManager();
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

    
    //Determine winner
    uint32_t winner = 0;
    uint32_t loser = 0;
    for(map<uint32_t, int>::iterator i = strength.begin(); i != strength.end(); ++i) {
        if(winner == 0) {
            winner = i->first;
        } else {
            if(strength[winner] < i->second) {
                winner = i->first;
            } else {
                loser = i->first;
            }
        }
    }

    //Remove losing combatants
    uint32_t losingRegion;
    uint32_t winningRegion;
    set<uint32_t> removedSystems;
    if(isInternal) {
        for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
            IGObject* ob = objectmanager->getObject(i->first);
            Fleet* f = (Fleet*) ob->getObjectBehaviour();
            if(f->getOwner() != winner) {
                sendHome(i->first);
                Player* p = playermanager->getPlayer(winner);
                p->setScore(2, p->getScore(2) + 1);
                losingRegion = i->second;
                winningRegion = losingRegion;
            }
        }
        objects = objectmanager->getAllIds();
        for(itcurr = objects.begin(); itcurr!= objects.end(); ++itcurr) {
            IGObject* ob = objectmanager->getObject(*itcurr);
            if(ob->getType() == obtm->getObjectTypeByName("Star System")) {
                StarSystem* sysData = (StarSystem*) ob->getObjectBehaviour();
                if(sysData->getRegion() == losingRegion) {
                    sysData->setRegion(0);
                    removedSystems.insert(*itcurr);
                }
            }
        }
    } else {
        string shipType;
        for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
            IGObject* ob = objectmanager->getObject(i->first);
            Fleet* f = (Fleet*) ob->getObjectBehaviour();
            if(f->getOwner() != winner) {
                losingRegion = i->second;
                shipType = ob->getName();
                sendHome(i->first);
            } else {
                winningRegion = i->second;
            }
        }

        int resourceType;
        if(shipType.compare("Merchant Leader") == 0) {
            resourceType = 4;
        } else if(shipType.compare("Scientist Leader") == 0) {
            resourceType = 5;
        } else if(shipType.compare("Settler Leader") == 0) {
            resourceType = 6;
        } else {
            resourceType = 7;
        }

        Player* player = playermanager->getPlayer(winner);
        player->setScore(resourceType - 3, player->getScore(resourceType-3) + 1);

        objects = objectmanager->getAllIds();
        for(itcurr = objects.begin(); itcurr!= objects.end(); ++itcurr) {
            IGObject* ob = objectmanager->getObject(*itcurr);
            if(ob->getType() == obtm->getObjectTypeByName("Planet")) {
                IGObject* sys = objectmanager->getObject(ob->getParent());
                StarSystem* sysData = (StarSystem*) sys->getObjectBehaviour();
                if(sysData->getRegion() == losingRegion) {
                    Planet* p = (Planet*) ob->getObjectBehaviour();
                    sysData->setRegion(0);
                    if(p->getResource(resourceType) > 0) {
                        p->removeResource(resourceType, 1);
                        player->setScore(resourceType - 3, player->getScore(resourceType-3) + 1);
                    } else {
                        removedSystems.insert(sys->getID());
                    }
                } else if(sysData->getRegion() == winningRegion) {
                    sysData->setRegion(0);
                    removedSystems.insert(sys->getID());
                }
            }
        }
    }
   
    //rebuild region
    for(itcurr = removedSystems.begin(); itcurr!= removedSystems.end(); ++itcurr) {
        rebuildRegion(*itcurr);
    }
 
    //TODO: Check for combat

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
    strength.clear();
    for(map<uint32_t, uint32_t>::iterator i = combatants.begin(); i != combatants.end(); ++i) {
        IGObject* ob = Game::getGame()->getObjectManager()->getObject(i->first);
        Fleet* f = (Fleet*) ob->getObjectBehaviour();
        strength[f->getOwner()] = 0;
    }
}

void TaeTurn::addReinforcement(uint32_t player) {
    //+1 to player's combat strength for this turn
    strength[player] += 1;
    Logger::getLogger()->debug("Add Reinforcement");
}

void TaeTurn::sendHome(uint32_t fleet) {
    Game* game = Game::getGame();
    ObjectManager* obm = game->getObjectManager();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    PlayerManager* pm = game->getPlayerManager();

    IGObject* fleetobj = obm->getObject(fleet);
    //Check to make sure it is really a fleet
    if(fleetobj->getType() != obtm->getObjectTypeByName("Fleet")) {
        return;
    }
    
    //Get all the required objects
    Fleet* f = (Fleet*) fleetobj->getObjectBehaviour();
    Player* p = pm->getPlayer(f->getOwner());
    IGObject* sys = obm->getObject(fleetobj->getParent());
    StarSystem* sysData = (StarSystem*) sys->getObjectBehaviour();

    //Remove fleet from system
    sysData->setRegion(0);
    fleetobj->removeFromParent();

    //Find it's home planet
    std::set<uint32_t> objects = obm->getAllIds();
    std::set<uint32_t>::iterator itcurr;
    for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject * ob = obm->getObject(*itcurr);
        if(ob->getName().compare(string(p->getName() + "'s Home Planet")) == 0) {
            Planet* p = (Planet*) ob->getObjectBehaviour();
            f->setPosition(p->getPosition());
            fleetobj->addToParent(ob->getID());
        }
    }
}

void TaeTurn::rebuildRegion(uint32_t system) {
    Game* game = Game::getGame();
    ObjectManager* obm = game->getObjectManager();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    set<uint32_t>::iterator itcurr;

    IGObject* sys = obm->getObject(system);
    StarSystem* sysData = (StarSystem*) sys->getObjectBehaviour();

    //Check to make sure it doesnt already have a region
    if(sysData->getRegion() != 0) {
        return;
    }

    //Check to make sure it is colonized or occupied
    set<uint32_t> children = sys->getContainedObjects();
    for(itcurr = children.begin(); itcurr != children.end(); itcurr++) {
        IGObject* ob = obm->getObject(*itcurr);
        bool resource = true;
        bool occupied = false;
        if(ob->getType() == obtm->getObjectTypeByName("Planet")) {
            Planet* p = (Planet*) ob->getObjectBehaviour();
            if(p->getResource(4) == 0 && p->getResource(5) == 0 && p->getResource(6) == 0 && p->getResource(7) == 0) {
                resource = false;
            }
        } else if(ob->getType() == obtm->getObjectTypeByName("Fleet")) {
            occupied = true;
        }
        if(!(resource || occupied)) {
            return;
        }
    }

    //Check to make sure this isnt a home system
    if(sys->getName().find("'s System") != string::npos) {
        return;
    }
    
    //Get neighbors
    set<uint32_t> regions;
    set<uint32_t> emptyNeighbors;
    Vector3d pos = sysData->getPosition();
    //east-west neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(80000*i,0,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject *tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Star System")) {
                uint32_t r = ((StarSystem*)(tempObj->getObjectBehaviour()))->getRegion();
                if(r == 0) {
                    emptyNeighbors.insert(*j);
                } else if (regions.count(r) == 0) {
                    regions.insert(r);
                }
            }
        }
    }
    //north-south neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(0,80000*i,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject *tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Star System")) {
                uint32_t r = ((StarSystem*)(tempObj->getObjectBehaviour()))->getRegion();
                if(r == 0) {
                    emptyNeighbors.insert(*j);
                } else if (regions.count(r) == 0) {
                    regions.insert(r);
                }
            }
        }
    }

    //Set Region
    if(regions.size() == 0) {
        sysData->setRegion(sys->getID());
        stringstream out;
        out << sysData->getRegion();
        Logger::getLogger()->debug(string("System " + sys->getName() + " added to region " + out.str()).c_str());
    } else if(regions.size() == 1) {
        sysData->setRegion(*(regions.begin()));
        stringstream out;
        out << sysData->getRegion();
        Logger::getLogger()->debug(string("System " + sys->getName() + " added to region " + out.str()).c_str());
    } else {
        Logger::getLogger()->debug(string("** Unable to rebuild region! System, " + sys->getName() + " is bordering more than one region! ***").c_str());
    }

    //Recurse on neighbors
    for(itcurr = emptyNeighbors.begin(); itcurr != emptyNeighbors.end(); itcurr++) {
        rebuildRegion(*itcurr);
    }
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
