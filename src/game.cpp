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
  //if new, create new player

  Player * newplayer = new Player();
  newplayer->setName(name);
  newplayer->setPass(pass);
  players.push_back(newplayer);
  return newplayer;
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

