#include <iostream>
#include <stdlib.h>
#include <ctime>

#include "logging.h"
#include "player.h"
#include "object.h"
#include "objectdata.h"
#include "ownedobject.h"
#include "frame.h"
#include "net.h"

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
	universe->setType(0);
	universe->setName("The Universe");
	universe->setPosition3(0ll, 0ll, 0ll);
	universe->setVelocity3(0ll, 0ll, 0ll);

	//add contained objects
	IGObject *mw_galaxy = new IGObject();
	objects[mw_galaxy->getID()] = mw_galaxy;
	mw_galaxy->setSize(10000000000ll);
	mw_galaxy->setType(1);
	mw_galaxy->setName("Milky Way Galaxy");
	mw_galaxy->setPosition3(0ll, -6000ll, 0ll);
	mw_galaxy->setVelocity3(0ll, 1000ll, 0ll);
	
	universe->addContainedObject(mw_galaxy->getID());
	// star system 1
	IGObject *sol = new IGObject();
	objects[sol->getID()] = sol;
	sol->setSize(1400000ll);
	sol->setType(2);
	sol->setName("Sol/Terra System");
	sol->setPosition3(3000000000ll, 2000000000ll, 0ll);
	sol->setVelocity3(-1500000ll, 1500000ll, 0ll);
	
	mw_galaxy->addContainedObject(sol->getID());
	// star system 2
	IGObject *ac = new IGObject();
	objects[ac->getID()] = ac;
	ac->setSize(800000ll);
	ac->setType(2);
	ac->setName("Alpha Centauri System");
	ac->setPosition3(-1500000000ll, 1500000000ll, 0ll);
	ac->setVelocity3(-1000000ll, -1000000ll, 0ll);
	
	mw_galaxy->addContainedObject(ac->getID());
	// star system 3
	IGObject *sirius = new IGObject();
	objects[sirius->getID()] = sirius;
	sirius->setSize(2000000ll);
	sirius->setType(2);
	sirius->setName("Sirius System");
	sirius->setPosition3(-250000000ll, -4000000000ll, 0ll);
	sirius->setVelocity3(2300000ll, 0ll, 0ll);
	
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

	std::list < Player * >::iterator itcurr, itend;

	itend = players.end();
	for (itcurr = players.begin(); itcurr != itend; itcurr++) {
		char *itname = (*itcurr)->getName();
		if (strncmp(name, itname, strlen(name) + 1) == 0) {
			char *itpass = (*itcurr)->getPass();
			if (strncmp(pass, itpass, strlen(pass) + 1) == 0) {
				rtn = (*itcurr);
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
		players.push_back(rtn);

		// HACK
		// adds a star system, planet and fleet, owned by the player
		IGObject *star = new IGObject();
		objects[star->getID()] = star;
		star->setSize(2000000ll);
		star->setType(2);
		star->setName("Unknown Star System");
		star->setPosition3((long long)(((rand() % 1000) - 500) * 10000000),
				   (long long)(((rand() % 1000) - 500) * 10000000),
				   (long long)(((rand() % 1000) - 500) * 10000000));
		star->setVelocity3(0ll, 0ll, 0ll);
		
		getObject(1)->addContainedObject(star->getID());

		IGObject *planet = new IGObject();
		objects[planet->getID()] = planet;
		planet->setSize(2);
		planet->setType(3);
		planet->setName("A planet");
		((OwnedObject*)(planet->getObjectData()))->setOwner(rtn->getID());
		planet->setPosition3(star->getPositionX() + (long long)((rand() % 10000) - 5000),
				     star->getPositionY() + (long long)((rand() % 10000) - 5000),
				     star->getPositionZ() + (long long)((rand() % 10000) - 5000));
		planet->setVelocity3(0LL, 0ll, 0ll);
		planet->addAction(-1, rtn->getID(), odT_Build);
		
		star->addContainedObject(planet->getID());

		IGObject *fleet = new IGObject();
		objects[fleet->getID()] = fleet;
		fleet->setSize(2);
		fleet->setType(4);
		fleet->setName("A fleet");
		((OwnedObject*)(fleet->getObjectData()))->setOwner(rtn->getID());
		fleet->setPosition3(star->getPositionX() + (long long)((rand() % 10000) - 5000),
				     star->getPositionY() + (long long)((rand() % 10000) - 5000),
				     star->getPositionZ() + (long long)((rand() % 10000) - 5000));
		fleet->setVelocity3(0LL, 0ll, 0ll);
	
		fleet->addAction(-1, rtn->getID(), odT_Move);
		fleet->addAction(-1, rtn->getID(), odT_Nop);
		star->addContainedObject(fleet->getID());
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

std::list <unsigned int> Game::getObjectsByPos(long long x, long long y, long long z, unsigned long long r)
{
  std::list <unsigned int> oblist;

  std::map<unsigned int, IGObject *>::iterator itcurr = objects.begin();

  for( ; itcurr != objects.end(); ++itcurr) {
    long long dx = itcurr->second->getPositionX() - x;
    long long dy = itcurr->second->getPositionY() - y;
    long long dz = itcurr->second->getPositionZ() - z;
    unsigned long long br = itcurr->second->getSize() / 2;
    long long diff = (unsigned long long)(dx * dx) + (unsigned long long)(dy * dy) + (unsigned long long)(dz * dz) - r * r - br * br;
    if(diff <= 0)
      oblist.push_back(itcurr->first);

  }
  
  return oblist;
}

std::list <unsigned int> Game::getContainerByPos(long long x, long long y, long long z){
  std::list<unsigned int> oblist;

  for(std::map<unsigned int, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr){
    long long dx = itcurr->second->getPositionX() - x;
    long long dy = itcurr->second->getPositionY() - y;
    long long dz = itcurr->second->getPositionZ() - z;
    unsigned long long br = itcurr->second->getSize() / 2;

    long long diff = (unsigned long long)(dx * dx) + (unsigned long long)(dy * dy) + (unsigned long long)(dz * dz) - br * br;
    if(diff <= 0)
      oblist.push_back(itcurr->first);
  }
  
  return oblist;
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

}

Game::Game(Game & rhs)
{

}

Game::~Game()
{

}

Game Game::operator=(Game & rhs)
{

}
