/*  Game controller for tpserver-cpp
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <cassert>

#include "logging.h"
#include "player.h"
#include "object.h"
#include "order.h"
#include "universe.h"
#include "frame.h"
#include "net.h"
#include "vector3d.h"
#include "objectmanager.h"
#include "ordermanager.h"
#include "objectdatamanager.h"
#include "boardmanager.h"
#include "playermanager.h"
#include "ownedobject.h"
#include "combatstrategy.h"
#include "designstore.h"
#include "ruleset.h"
#include "persistence.h"

#include "game.h"

Game *Game::myInstance = NULL;

Game *Game::getGame()
{
	if (myInstance == NULL) {
		myInstance = new Game();
	}
	return myInstance;
}

bool Game::setRuleset(Ruleset* rs){
  if(!loaded){
    if(ruleset != NULL)
      delete ruleset;
    ruleset = rs;
    return true;
  }else{
    return false;
  }
}

Ruleset* Game::getRuleset() const{
  return ruleset;
}

bool Game::load()
{
  if(!loaded && ruleset != NULL){
    Logger::getLogger()->info("Loading Game");  

    ruleset->initGame();

    //if nothing loaded from database
    //init game
        IGObject* universe = objectmanager->getObject(0);
        if(universe != NULL){
            objectmanager->doneWithObject(0);
            objectmanager->init();
            ordermanager->init();
            boardmanager->init();
            playermanager->init();
            designstore->init();
        }else{
            Logger::getLogger()->info("Creating Game");
            ruleset->createGame();
        }
    
    loaded = true;
    return true;
  }else{
    Logger::getLogger()->warning("Game already loaded");
    return false;
  }

}

bool Game::start(){
  if(loaded && !started){
    Logger::getLogger()->info("Starting Game");

    ruleset->startGame();

    resetEOTTimer();

    started = true;
    return true;
  }else{
    Logger::getLogger()->warning("Game not starting, not loaded or already started");
    return false;
  }
}


void Game::save()
{
	Logger::getLogger()->info("Game saved");
}

ObjectManager* Game::getObjectManager() const{
    return objectmanager;
}

OrderManager* Game::getOrderManager() const{
  return ordermanager;
}

ObjectDataManager* Game::getObjectDataManager() const{
  return objectdatamanager;
}

BoardManager* Game::getBoardManager() const{
    return boardmanager;
}

PlayerManager* Game::getPlayerManager() const{
    return playermanager;
}

CombatStrategy* Game::getCombatStrategy() const{
  return combatstrategy;
}

void Game::setCombatStrategy(CombatStrategy* cs){
  if(combatstrategy != NULL)
    delete combatstrategy;
  combatstrategy = cs;
}

DesignStore* Game::getDesignStore() const{
  return designstore;
}

Persistence* Game::getPersistence() const{
    return persistence;
}

void Game::setPersistence(Persistence* p){
    persistence = p;
}

bool Game::isLoaded() const{
  return loaded;
}

bool Game::isStarted() const{
  return (loaded && started);
}

void Game::doEndOfTurn()
{
    if(loaded && started){
        Logger::getLogger()->info("End Of Turn started");

        // send frame to all connections that the end of turn has started
        Frame * frame = new Frame(fv0_2);
        frame->setType(ft02_Time_Remaining);
        frame->packInt(0);
        Network::getNetwork()->sendToAll(frame);

	// DO END OF TURN STUFF HERE
	std::set<uint32_t>::iterator itcurr;

        //do orders
        std::set<uint32_t> objects = ordermanager->getObjectsWithOrders();
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = objectmanager->getObject(*itcurr);
            Order * currOrder = ordermanager->getFirstOrder(ob);
            if(currOrder != NULL){
                if(currOrder->doOrder(ob)){
                ordermanager->removeFirstOrder(ob);
                }else{
                    ordermanager->updateFirstOrder(ob);
                }
            }
            objectmanager->doneWithObject(ob->getID());
	}

	objectmanager->clearRemovedObjects();

	// update positions and velocities
        objects = objectmanager->getAllIds();
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = objectmanager->getObject(*itcurr);
            ob->updatePosition();
            objectmanager->doneWithObject(ob->getID());
        }

	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = objectmanager->getObject(*itcurr);
            //ob->updatePosition();
            std::set<unsigned int> cont = ob->getContainedObjects();
            for(std::set<uint32_t>::iterator ita = cont.begin(); ita != cont.end(); ++ita){
                IGObject* itaobj = objectmanager->getObject(*ita);
                if(itaobj->getType() == obT_Fleet || (itaobj->getType() == obT_Planet && ((OwnedObject*)(itaobj->getObjectData()))->getOwner() != -1)){
                    for(std::set<unsigned int>::iterator itb = ita; itb != cont.end(); ++itb){
                        IGObject* itbobj = objectmanager->getObject(*itb);
                        if((*ita != *itb) && (itbobj->getType() == obT_Fleet || (itbobj->getType() == obT_Planet && ((OwnedObject*)(itbobj->getObjectData()))->getOwner() != 0))){
                            if(((OwnedObject*)(itaobj->getObjectData()))->getOwner() != ((OwnedObject*)(itbobj->getObjectData()))->getOwner()){
                                uint64_t diff = itaobj->getPosition().getDistance(itbobj->getPosition());
                                if(diff <= itaobj->getSize() / 2 + itbobj->getSize() / 2){
                                    combatstrategy->setCombatants(itaobj, itbobj);
                                    combatstrategy->doCombat();
                                    if(!combatstrategy->isAliveCombatant1()){
                                        if(itaobj->getType() == obT_Planet){
                                            ((OwnedObject*)(itaobj->getObjectData()))->setOwner(0);
                                        }else{
                                            objectmanager->scheduleRemoveObject(*ita);
                                        }
                                    }
                                    if(!combatstrategy->isAliveCombatant2()){
                                        if(itbobj->getType() == obT_Planet){
                                            ((OwnedObject*)(itbobj->getObjectData()))->setOwner(-1);
                                        }else{
                                            objectmanager->scheduleRemoveObject(*itb);
                                        }
                                    }
                                }
                            }
                        }
                        objectmanager->doneWithObject(itbobj->getID());
                    }
                }
                objectmanager->doneWithObject(itaobj->getID());
            }
            objectmanager->doneWithObject(ob->getID());
        }

        objectmanager->clearRemovedObjects();
	
	// to once a turn (right at the end)
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = objectmanager->getObject(*itcurr);
	  ob->getObjectData()->doOnceATurn(ob);
	  objectmanager->doneWithObject(ob->getID());
	}

	// find the objects that are visible to each player
	std::set<uint32_t> vis = objectmanager->getAllIds();
        std::set<uint32_t> players = playermanager->getAllIds();
	for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
            playermanager->getPlayer(*itplayer)->setVisibleObjects(vis);
	}
        playermanager->updateAll();

	// increment the time to the next turn
	turnTime += turnIncrement;
        if(secondsToEOT() <= 0)
            resetEOTTimer();

	// send frame to all connections that the end of turn has finished
	frame = new Frame(fv0_2);
	frame->setType(ft02_Time_Remaining);
	frame->packInt(secondsToEOT());
	Network::getNetwork()->sendToAll(frame);

	Logger::getLogger()->info("End Of Turn finished");
    }else{
        Logger::getLogger()->info("End Of Turn not run because game not started");
        turnTime += turnIncrement;
    }
}

void Game::resetEOTTimer(){
  turnTime = time(NULL) + turnIncrement;
}

int Game::secondsToEOT(){
  return turnTime - time(NULL);
}

int Game::getTurnNumber(){
    int turnnum = ((Universe*)(objectmanager->getObject(0)->getObjectData()))->getYear();
    objectmanager->doneWithObject(0);
    return turnnum;
}

void Game::setTurnLength(unsigned int sec){
  turnIncrement = sec;
}

void Game::saveAndClose()
{
	save();
	//remove and delete players

	//remove and delete objects

	Logger::getLogger()->info("Game saved & closed");
}

Game::Game()
{
    objectmanager = new ObjectManager();
  ordermanager = new OrderManager();
  objectdatamanager = new ObjectDataManager();
    boardmanager = new BoardManager();
    playermanager = new PlayerManager();
  designstore = new DesignStore();
  combatstrategy = NULL;
  ruleset = NULL;
    persistence = NULL;
  loaded = false;
  started = false;

  turnIncrement = 86400; //24 hours
  resetEOTTimer();
  //this is a good place to seed the PNRG
  srand((getpid() + time(NULL)) % RAND_MAX);
}

Game::Game(Game & rhs)
{

}

Game::~Game()
{
    delete objectmanager;
  delete ordermanager;
  delete objectdatamanager;
    delete boardmanager;
    delete playermanager;
  if(combatstrategy != NULL)
    delete combatstrategy;
  if(ruleset != NULL)
    delete ruleset;
  delete designstore;
    if(persistence != NULL)
        delete persistence;
}

Game Game::operator=(Game & rhs)
{
  //only here to stop people doing funny things...
  assert(0);
  return *this;
}
