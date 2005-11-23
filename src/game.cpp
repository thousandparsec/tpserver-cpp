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
#include "universe.h"
#include "frame.h"
#include "net.h"
#include "vector3d.h"
#include "ordermanager.h"
#include "objectdatamanager.h"
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
    ruleset->createGame();
    
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

Player *Game::findPlayer(char *name, char *pass)
{
	Logger::getLogger()->debug("finding player");

	//look for current/known players
	Player *rtn = NULL;

	// hack HACK!!
	if (strcmp("guest", name) == 0 && strcmp("guest", pass) == 0)
		return rtn;
	// end of hack HACK!!

	std::map<unsigned int, Player*>::iterator itcurr;

	for (itcurr = players.begin(); itcurr != players.end(); ++itcurr) {
		char *itname = (*itcurr).second->getName();
		if (strncmp(name, itname, strlen(name) + 1) == 0) {
			char *itpass = (*itcurr).second->getPass();
			if (strncmp(pass, itpass, strlen(pass) + 1) == 0) {
				rtn = (*itcurr).second;
			}
			delete itpass;
		}
		delete itname;
		if (rtn != NULL)
			break;
	}

	if (rtn == NULL) {
		//if new, create new player

		rtn = new Player();
		rtn->setName(name);
		rtn->setPass(pass);

		if(ruleset->onAddPlayer(rtn)){
		  // player can be added
		  players[rtn->getID()] = (rtn);
		  
		  ruleset->onPlayerAdded(rtn);
		  rtn->setVisibleObjects(getObjectIds());

		}else{
		  // player can not be added
		  delete rtn;
		  rtn = NULL;
		}
		
	}
	return rtn;
}

Player* Game::getPlayer(unsigned int id){
  Player* rtn = NULL;
  std::map<unsigned int, Player*>::iterator pl = players.find(id);
  if(pl != players.end()){
    rtn = (*pl).second;
  }
  return rtn;
}

std::set<unsigned int> Game::getPlayerIds() const{
  std::set<unsigned int> vis;
  for(std::map<unsigned int, Player*>::const_iterator itid = players.begin();
      itid != players.end(); ++itid){
    vis.insert(itid->first);
  }
  return vis;
}

IGObject *Game::getObject(unsigned int id)
{
	if (id == 0) {
		return universe;
	}
	IGObject *rtn = NULL;
	std::map < unsigned int, IGObject * >::iterator obj = objects.find(id);
	if (obj != objects.end()) {
		rtn = (*obj).second;
	}
	return rtn;
	//may need more work
}

void Game::addObject(IGObject* obj)
{
  objects[obj->getID()] = obj;
  if(obj->getID() == 0){
    universe = obj;
  }
}

void Game::scheduleRemoveObject(unsigned int id){
  scheduleRemove.insert(id);
}

std::list <unsigned int> Game::getObjectsByPos(const Vector3d & pos, unsigned long long r)
{
  std::list <unsigned int> oblist;

  std::map<unsigned int, IGObject *>::iterator itcurr = objects.begin();

  for( ; itcurr != objects.end(); ++itcurr) {
    unsigned long long br = itcurr->second->getSize() / 2;
    unsigned long long diff = itcurr->second->getPosition().getDistance(pos); /*- r - br;*/
    if(diff <=  r + br)
      oblist.push_back(itcurr->first);

  }
  
  return oblist;
}

std::list <unsigned int> Game::getContainerByPos(const Vector3d & pos){
  std::list<unsigned int> oblist;

  for(std::map<unsigned int, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr){
    if(itcurr->second->getContainerType() >= 1){
      unsigned long long br = itcurr->second->getSize() / 2 + itcurr->second->getSize() % 2;
      
      //long long diff = itcurr->second->getPosition().getDistanceSq(pos) - br * br;
      if((unsigned long long)(itcurr->second->getPosition().getDistance(pos)) <= br)
	oblist.push_front(itcurr->first);
    }
  }
  
  return oblist;
}

std::set<unsigned int> Game::getObjectIds() const{
  std::set<unsigned int> vis;
  for(std::map<unsigned int, IGObject*>::const_iterator itid = objects.begin();
      itid != objects.end(); ++itid){
    vis.insert(itid->first);
  }
  return vis;
}

OrderManager* Game::getOrderManager() const{
  return ordermanager;
}

ObjectDataManager* Game::getObjectDataManager() const{
  return objectdatamanager;
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
	std::map<unsigned int, IGObject *>::iterator itcurr;
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = itcurr->second;
	  Order * currOrder = ob->getFirstOrder();
	  if(currOrder != NULL){
	    if(currOrder->doOrder(ob)){
	      ob->removeFirstOrder();
	    }
	  }
	}

	std::set<unsigned int>::iterator itrm;
	for(itrm = scheduleRemove.begin(); itrm != scheduleRemove.end(); ++itrm){
	  objects[*itrm]->removeFromParent();
	  delete objects[*itrm];
	  objects.erase(*itrm);
	}
	scheduleRemove.clear();

	// update positions and velocities
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = itcurr->second;
	  ob->updatePosition();
	  
	}

	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = itcurr->second;
	  //ob->updatePosition();
	  std::set<unsigned int> cont = ob->getContainedObjects();
	  for(std::set<unsigned int>::iterator ita = cont.begin(); ita != cont.end(); ++ita){
	    if(objects[(*ita)]->getType() == obT_Fleet || (objects[(*ita)]->getType() == obT_Planet && ((OwnedObject*)(objects[(*ita)])->getObjectData())->getOwner() != -1)){
	      for(std::set<unsigned int>::iterator itb = ita; itb != cont.end(); ++itb){
		if((*ita != *itb) && (objects[(*itb)]->getType() == obT_Fleet || (objects[(*itb)]->getType() == obT_Planet && ((OwnedObject*)(objects[(*itb)])->getObjectData())->getOwner() != -1))){
		  if(((OwnedObject*)(objects[(*ita)]->getObjectData()))->getOwner() != ((OwnedObject*)(objects[(*itb)]->getObjectData()))->getOwner()){
		    unsigned long long diff = objects[(*ita)]->getPosition().getDistance(objects[(*itb)]->getPosition());
		    if(diff <= objects[(*ita)]->getSize() / 2 + objects[(*itb)]->getSize() / 2){
		      combatstrategy->setCombatants(objects[(*ita)], objects[(*itb)]);
		      combatstrategy->doCombat();
		      if(!combatstrategy->isAliveCombatant1()){
			if(objects[(*ita)]->getType() == obT_Planet){
			  ((OwnedObject*)(objects[(*ita)]->getObjectData()))->setOwner(-1);
			}else{
			  scheduleRemove.insert(*ita);
			}
		      }
		      if(!combatstrategy->isAliveCombatant2()){
			if(objects[(*itb)]->getType() == obT_Planet){
			  ((OwnedObject*)(objects[(*itb)]->getObjectData()))->setOwner(-1);
			}else{
			  scheduleRemove.insert(*itb);
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}

	for(itrm = scheduleRemove.begin(); itrm != scheduleRemove.end(); ++itrm){
	  objects[*itrm]->removeFromParent();
	  delete objects[*itrm];
	  objects.erase(*itrm);
	}
	scheduleRemove.clear();
	
	// to once a turn (right at the end)
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = itcurr->second;
	  ob->getObjectData()->doOnceATurn(ob);
	  
	}

	// find the objects that are visible to each player
	std::set<unsigned int> vis = getObjectIds();
	
	for(std::map<unsigned int, Player*>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
	  (itplayer->second)->setVisibleObjects(vis);
	}

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
  return ((Universe*)(universe->getObjectData()))->getYear();
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
  ordermanager = new OrderManager();
  objectdatamanager = new ObjectDataManager();
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
  delete ordermanager;
  delete objectdatamanager;
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
