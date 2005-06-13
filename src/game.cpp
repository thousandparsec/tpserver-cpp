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
#include "objectdata.h"
#include "ownedobject.h"
#include "fleet.h"
#include "universe.h"
#include "frame.h"
#include "net.h"
#include "vector3d.h"
#include "ordermanager.h"
#include "objectdatamanager.h"
#include "combatstrategy.h"
#include "rspcombat.h"
#include "designstore.h"

#include "game.h"

Game *Game::myInstance = NULL;

Game *Game::getGame()
{
	if (myInstance == NULL) {
		myInstance = new Game();
	}
	return myInstance;
}

void Game::createRandomUniverse()
{
	Logger::getLogger()->info("Creating random universe");
}

void Game::createRealUniverse()
{
	Logger::getLogger()->info("Creating real universe");
}

void Game::createTutorial()
{
	Logger::getLogger()->info("Creating tutorial");
	objects.clear();
	universe = new IGObject();
	objects[universe->getID()] = universe;
	universe->setSize(100000000000ll);
	universe->setType(obT_Universe);
	universe->setName("The Universe");
	universe->setPosition(Vector3d(0ll, 0ll, 0ll));
	universe->setVelocity(Vector3d(0ll, 0ll, 0ll));

	//add contained objects
	IGObject *mw_galaxy = new IGObject();
	objects[mw_galaxy->getID()] = mw_galaxy;
	mw_galaxy->setSize(10000000000ll);
	mw_galaxy->setType(obT_Galaxy);
	mw_galaxy->setName("Milky Way Galaxy");
	mw_galaxy->setPosition(Vector3d(0ll, -6000ll, 0ll));
	mw_galaxy->setVelocity(Vector3d(0ll, 0ll, 0ll));
	
	universe->addContainedObject(mw_galaxy->getID());
	// star system 1
	IGObject *sol = new IGObject();
	objects[sol->getID()] = sol;
	sol->setSize(1400000ll);
	sol->setType(obT_Star_System);
	sol->setName("Sol/Terra System");
	sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
	sol->setVelocity(Vector3d(0ll, 0ll, 0ll));
	
	mw_galaxy->addContainedObject(sol->getID());
	// star system 2
	IGObject *ac = new IGObject();
	objects[ac->getID()] = ac;
	ac->setSize(800000ll);
	ac->setType(obT_Star_System);
	ac->setName("Alpha Centauri System");
	ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
	ac->setVelocity(Vector3d(0ll, 0ll, 0ll));
	
	mw_galaxy->addContainedObject(ac->getID());
	// star system 3
	IGObject *sirius = new IGObject();
	objects[sirius->getID()] = sirius;
	sirius->setSize(2000000ll);
	sirius->setType(obT_Star_System);
	sirius->setName("Sirius System");
	sirius->setPosition(Vector3d(-250000000ll, -4000000000ll, 0ll));
	sirius->setVelocity(Vector3d(0ll, 0ll, 0ll));
	
	mw_galaxy->addContainedObject(sirius->getID());

	// now for some planets

	IGObject *earth = new IGObject();
	objects[earth->getID()] = earth;
	earth->setSize(2);
	earth->setType(obT_Planet);
	earth->setName("Earth/Terra");
	earth->setPosition(sol->getPosition() + Vector3d(14960ll, 0ll, 0ll));
	sol->addContainedObject(earth->getID());

	IGObject *venus = new IGObject();
	objects[venus->getID()] = venus;
	venus->setSize(2);
	venus->setType(obT_Planet);
	venus->setName("Venus");
	venus->setPosition(sol->getPosition() + Vector3d(0ll, 10800ll, 0ll));
	sol->addContainedObject(venus->getID());

	IGObject *mars = new IGObject();
	objects[mars->getID()] = mars;
	mars->setSize(1);
	mars->setType(obT_Planet);
	mars->setName("Mars");
	mars->setPosition(sol->getPosition() + Vector3d(-22790ll, 0ll, 0ll));
	sol->addContainedObject(mars->getID());

	IGObject *acprime = new IGObject();
	objects[acprime->getID()] = acprime;
	acprime->setSize(2);
	acprime->setType(obT_Planet);
	acprime->setName("Alpha Centauri Prime");
	acprime->setPosition(ac->getPosition() + Vector3d(-6300ll, 78245ll, 0ll));
	ac->addContainedObject(acprime->getID());

	IGObject *s1 = new IGObject();
	objects[s1->getID()] = s1;
	s1->setSize(2);
	s1->setType(obT_Planet);
	s1->setName("Sirius 1");
	s1->setPosition(sirius->getPosition() + Vector3d(45925ll, -34262ll, 0ll));
	sirius->addContainedObject(s1->getID());

	combatstrategy = new RSPCombat();
	
	loaded = true;

	turnIncrement = 600; // 10 minutes
	resetEOTTimer();
	started = true;
}

/*
void Game::loadGame(char *file)
{


}
*/

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
		players[rtn->getID()] = (rtn);

		// HACK
		// adds a star system, planet and fleet, owned by the player
		IGObject *star = new IGObject();
		objects[star->getID()] = star;
		star->setSize(2000000ll);
		star->setType(obT_Star_System);
		char* temp = new char[strlen(name) + 13];
		strncpy(temp, name, strlen(name));
		strncpy(temp + strlen(name), " Star System", 12);
		temp[strlen(name) + 12] = '\0';
		star->setName(temp);
		delete[] temp;
		star->setPosition(Vector3d((long long)(((rand() % 1000) - 500) * 10000000),
					    (long long)(((rand() % 1000) - 500) * 10000000),
					   /*(long long)(((rand() % 1000) - 500) * 10000000)*/ 0));
		star->setVelocity(Vector3d(0ll, 0ll, 0ll));
		
		getObject(1)->addContainedObject(star->getID());

		IGObject *planet = new IGObject();
		objects[planet->getID()] = planet;
		planet->setSize(2);
		planet->setType(obT_Planet);
		temp = new char[strlen(name) + 8];
		strncpy(temp, name, strlen(name));
		strncpy(temp + strlen(name), " Planet", 7);
		temp[strlen(name) + 7] = '\0';
		planet->setName(temp);
		delete[] temp;
		((OwnedObject*)(planet->getObjectData()))->setOwner(rtn->getID());
		planet->setPosition(star->getPosition() + Vector3d((long long)((rand() % 10000) - 5000),
								   (long long)((rand() % 10000) - 5000),
								   /*(long long)((rand() % 10000) - 5000)*/ 0));
		planet->setVelocity(Vector3d(0LL, 0ll, 0ll));
				
		star->addContainedObject(planet->getID());

		IGObject *fleet = new IGObject();
		objects[fleet->getID()] = fleet;
		fleet->setSize(2);
		fleet->setType(obT_Fleet);
		temp = new char[strlen(name) + 13];
		strncpy(temp, name, strlen(name));
		strncpy(temp + strlen(name), " First Fleet", 12);
		temp[strlen(name) + 12] = '\0';
		fleet->setName(temp);
		delete[] temp;
		((OwnedObject*)(fleet->getObjectData()))->setOwner(rtn->getID());
		fleet->setPosition(star->getPosition() + Vector3d((long long)((rand() % 10000) - 5000),
								  (long long)((rand() % 10000) - 5000),
								  /*(long long)((rand() % 10000) - 5000)*/ 0));
		((Fleet*)(fleet->getObjectData()))->addShips(0, 2);
		fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
	
		star->addContainedObject(fleet->getID());

		std::set<unsigned int> vis;
		for(std::map<unsigned int, IGObject*>::iterator itid = objects.begin(); itid != objects.end(); ++itid){
		  vis.insert(itid->first);
		}
		rtn->setVisibleObjects(vis);
		
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

OrderManager* Game::getOrderManager() const{
  return ordermanager;
}

ObjectDataManager* Game::getObjectDataManager() const{
  return objectdatamanager;
}

CombatStrategy* Game::getCombatStrategy() const{
  return combatstrategy;
}

DesignStore* Game::getDesignStore(unsigned int id) const{
  std::map<unsigned int, DesignStore*>::const_iterator itcurr = designstores.find(id);
  if(itcurr != designstores.end()){
    return itcurr->second;
  }
  return NULL;
}

std::set<unsigned int> Game::getCategoryIds() const{
  std::set<unsigned int> set;
  for(std::map<unsigned int, DesignStore*>::const_iterator itcurr = designstores.begin();
      itcurr != designstores.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
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
	std::set<unsigned int> vis;
	for(std::map<unsigned int, IGObject*>::iterator itid = objects.begin(); itid != objects.end(); ++itid){
	  vis.insert(itid->first);
	}
	for(std::map<unsigned int, Player*>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
	  (itplayer->second)->setVisibleObjects(vis);
	}

	// increment the time to the next turn
	turnTime += turnIncrement;

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
  delete combatstrategy;
  while(!designstores.empty()){
    delete designstores.begin()->second;
    designstores.erase(designstores.begin());
  }
}

Game Game::operator=(Game & rhs)
{
  //only here to stop people doing funny things...
  assert(0);
  return *this;
}
