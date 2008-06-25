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

#include "planet.h"
#include "fleet.h"

#include "taeturn.h"

using std::stringstream;

TaeTurn::TaeTurn() : TurnProcess(), containerids(){

}

TaeTurn::~TaeTurn(){

}

void TaeTurn::doTurn(){
    std::set<uint32_t>::iterator itcurr;

    Game* game = Game::getGame();
    OrderManager* ordermanager = game->getOrderManager();
    ObjectManager* objectmanager = game->getObjectManager();
    PlayerManager* playermanager = game->getPlayerManager();


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
                        if(currOrder->doOrder(ob)){
                            orderqueue->removeFirstOrder();
                        }else{
                            orderqueue->updateFirstOrder();
                        }
                    }
                }
            }
        }

        if(ob->getContainerType() >= 1){
            containerids.insert(ob->getID());
        }
        objectmanager->doneWithObject(ob->getID());
    }

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
    std::set<uint32_t> players = playermanager->getAllIds();
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

        //Send end of turn message to each player
        Message * msg = new Message();
        msg->setSubject("Turn complete");
        stringstream out;
        out << "Your Current Score: \n";
        out << "Money: " << player->getScore(1) << "\n";
        out << "Technology: " << player->getScore(2) << "\n";
        out << "People: " << player->getScore(3) << "\n";
        out << "Raw Materials: " << player->getScore(4);
        msg->setBody(out.str());
        player->postToBoard(msg);
    }

    playermanager->updateAll();

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
