#include <iostream>

#include "logging.h"
#include "player.h"
#include "object.h"

#include "game.h"

Game* Game::myInstance = NULL;

Game* Game::getGame(){
  if(myInstance == NULL){
    myInstance = new Game();
  }
  return myInstance;
}

void Game::createRandomUniverse(){
  Logger::getLogger()->info("Creating random universe"); 
}

void Game::createRealUniverse(){
  Logger::getLogger()->info("Creating real universe");
}

void Game::createTutorial(){
  Logger::getLogger()->info("Creating tutorial");
  objects.clear();
  universe = new IGObject();
  objects.push_front(universe);
  universe->setSize(100000000000ll);
  universe->setType(0);
  universe->setName("The Universe");
  universe->setPosition3(1ll, 1000ll, 5ll);
  universe->setVelocity3(10ll, -2ll, 0ll);
  universe->setAcceleration3(0ll, 0ll, 10ll);
  //add contained objects
  IGObject *mw_galaxy = new IGObject();
  objects.push_back(mw_galaxy);
  mw_galaxy->setSize(1000000000ll);
  mw_galaxy->setType(1);
  mw_galaxy->setName("Milky Way Galaxy");
  mw_galaxy->setPosition3(0ll, 0ll, 0ll);
  mw_galaxy->setVelocity3(0ll, 1000ll, 500ll);
  mw_galaxy->setAcceleration3(0ll, 0ll, 0ll);
  universe->addContainedObject(mw_galaxy);
  
}

void Game::save(){
  Logger::getLogger()->info("Game saved"); 
}

Player* Game::findPlayer(char* name, char* pass){
  Logger::getLogger()->debug("finding player"); 
  
  //look for current/known players
  Player* rtn = NULL;

  // hack HACK!!
  if(strcmp("guest", name) == 0 && strcmp("guest", pass) == 0)
    return rtn;
  // end of hack HACK!!

  std::list<Player*>::iterator itcurr, itend;

  itend = players.end();
  for(itcurr = players.begin(); itcurr != itend; itcurr++){
    char* itname = (*itcurr)->getName();
    if(strncmp(name, itname, strlen(name) + 1) == 0){
      char* itpass = (*itcurr)->getPass();
      if(strncmp(pass, itpass, strlen(pass) + 1) == 0){
	rtn = (*itcurr);
      }
      delete itpass;
    }
    delete itname;
    if(rtn != NULL)
      break;
  }

  if(rtn == NULL){
    //if new, create new player
    
    rtn = new Player();
    rtn->setName(name);
    rtn->setPass(pass);
    players.push_back(rtn);
  }
  return rtn;
}

IGObject* Game::getObject(unsigned int id){
  if(id == 0){
    return universe;
  }

  return NULL;
  //ie: needs work
}

void Game::doEndOfTurn(){
  Logger::getLogger()->info("End Of Turn started"); 
}

void Game::saveAndClose(){
  save();
  //remove and delete players
  
  //remove and delete objects
  
  Logger::getLogger()->info("Game saved & closed"); 
}

Game::Game(){

}

Game::Game(Game &rhs){

}

Game::~Game(){

}

Game Game::operator=(Game &rhs){

}

