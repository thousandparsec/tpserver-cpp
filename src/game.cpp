#include <iostream>

#include "logging.h"
#include "player.h"

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
}

void Game::save(){
  Logger::getLogger()->info("Game saved"); 
}

Player* Game::findPlayer(char* name, char* pass){
  Logger::getLogger()->debug("finding player"); 
  
  //look for current/known players
  Player* rtn = NULL;

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

void Game::doEndOfTurn(){
  Logger::getLogger()->info("End Of Turn started"); 
}

void Game::saveAndClose(){
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

