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
#include "frame.h"
#include "net.h"
#include "vector3d.h"
#include "ordermanager.h"
#include "objectdatamanager.h"

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
	mw_galaxy->setVelocity(Vector3d(0ll, 1000ll, 0ll));
	
	universe->addContainedObject(mw_galaxy->getID());
	// star system 1
	IGObject *sol = new IGObject();
	objects[sol->getID()] = sol;
	sol->setSize(1400000ll);
	sol->setType(obT_Star_System);
	sol->setName("Sol/Terra System");
	sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
	sol->setVelocity(Vector3d(-1500000ll, 1500000ll, 0ll));
	
	mw_galaxy->addContainedObject(sol->getID());
	// star system 2
	IGObject *ac = new IGObject();
	objects[ac->getID()] = ac;
	ac->setSize(800000ll);
	ac->setType(obT_Star_System);
	ac->setName("Alpha Centauri System");
	ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
	ac->setVelocity(Vector3d(-1000000ll, -1000000ll, 0ll));
	
	mw_galaxy->addContainedObject(ac->getID());
	// star system 3
	IGObject *sirius = new IGObject();
	objects[sirius->getID()] = sirius;
	sirius->setSize(2000000ll);
	sirius->setType(obT_Star_System);
	sirius->setName("Sirius System");
	sirius->setPosition(Vector3d(-250000000ll, -4000000000ll, 0ll));
	sirius->setVelocity(Vector3d(2300000ll, 0ll, 0ll));
	
	mw_galaxy->addContainedObject(sirius->getID());

	turnIncrement = 600;
	turnTime = time(NULL) + 600;

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
		star->setName("Unknown Star System");
		star->setPosition(Vector3d((long long)(((rand() % 1000) - 500) * 10000000),
					    (long long)(((rand() % 1000) - 500) * 10000000),
					   /*(long long)(((rand() % 1000) - 500) * 10000000)*/ 0));
		star->setVelocity(Vector3d(0ll, 0ll, 0ll));
		
		getObject(1)->addContainedObject(star->getID());

		IGObject *planet = new IGObject();
		objects[planet->getID()] = planet;
		planet->setSize(2);
		planet->setType(obT_Planet);
		planet->setName("A planet");
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
		fleet->setName("A fleet");
		((OwnedObject*)(fleet->getObjectData()))->setOwner(rtn->getID());
		fleet->setPosition(star->getPosition() + Vector3d((long long)((rand() % 10000) - 5000),
								  (long long)((rand() % 10000) - 5000),
								  /*(long long)((rand() % 10000) - 5000)*/ 0));
		((Fleet*)(fleet->getObjectData()))->addShips(0, 2);
		fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
	
		star->addContainedObject(fleet->getID());
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
    long long diff = itcurr->second->getPosition().getDistanceSq(pos) - r * r - br * br;
    if(diff <= 0)
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
      if(itcurr->second->getPosition().getDistanceSq(pos) <= br * br)
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

void Game::doEndOfTurn()
{
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
	
	// to once a turn (right at the end)
	for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
	  IGObject * ob = itcurr->second;
	  ob->getObjectData()->doOnceATurn(ob);
	  
	}

	// increment the time to the next turn
	turnTime += turnIncrement;

	// send frame to all connections that the end of turn has finished
	frame = new Frame(fv0_2);
	frame->setType(ft02_Time_Remaining);
	frame->packInt(secondsToEOT());
	Network::getNetwork()->sendToAll(frame);

	Logger::getLogger()->info("End Of Turn finished");
}

void Game::resetEOTTimer(){
  turnTime = time(NULL) + turnIncrement;
}

int Game::secondsToEOT(){
  return turnTime - time(NULL);
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
}

Game::Game(Game & rhs)
{

}

Game::~Game()
{
  delete ordermanager;
  delete objectdatamanager;
}

Game Game::operator=(Game & rhs)
{
  //only here to stop people doing funny things...
  assert(0);
  return *this;
}
