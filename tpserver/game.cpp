/*  Game controller for tpserver-cpp
 *
 *  Copyright (C) 2003-2006, 2007  Lee Begg and the Thousand Parsec Project
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
#include <sstream>
#include <iomanip>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef VERSION
#define VERSION "0.0.0"
#endif
#endif

#include "logging.h"
#include "player.h"
#include "object.h"
#include "order.h"
#include "frame.h"
#include "net.h"
#include "vector3d.h"
#include "objectmanager.h"
#include "ordermanager.h"
#include "objectdatamanager.h"
#include "boardmanager.h"
#include "resourcemanager.h"
#include "playermanager.h"
#include "designstore.h"
#include "ruleset.h"
#include "persistence.h"
#include "tpscheme.h"
#include "settings.h"
#include "timercallback.h"
#include "prng.h"
#include "turnprocess.h"
#include "advertiser.h"

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
  if(!loaded && ruleset != NULL && tpscheme != NULL){
    Logger::getLogger()->info("Loading Game");  

    if(persistence->init()){
      Logger::getLogger()->debug("Persistence initialised");
    }else{
      Logger::getLogger()->error("Problem initialising Persistence, game not loaded.");
      return false;
    }

    ruleset->initGame();

    //if nothing loaded from database
    //init game
    if(!persistence->retrieveGameInfo()){
      Logger::getLogger()->info("Creating Game");
      ctime = time(NULL);
      turnNum = 0;
      std::ostringstream formater;
      formater << std::ios::hex << random->getInt32() << std::ios::hex << random->getInt32();
      key = formater.str();
      persistence->saveGameInfo();
      ruleset->createGame();
    }else{
      objectmanager->init();
      ordermanager->init();
      boardmanager->init();
      resourcemanager->init();
      playermanager->init();
      designstore->init();
    }
    
    loaded = true;
    return true;
  }else{
    if(loaded){
      Logger::getLogger()->warning("Game already loaded");
    }
    if(ruleset == NULL){
      Logger::getLogger()->warning("Game not loaded, no ruleset set");
    }
    if(tpscheme == NULL){
      Logger::getLogger()->warning("Game not loaded, no TpScheme implementation set");
    }
    return false;
  }

}

bool Game::start(){
  if(loaded && !started){
    Logger::getLogger()->info("Starting Game");

    ruleset->startGame();
    
    uint32_t tl = atoi(Settings::getSettings()->get("turn_length").c_str());
    if(tl != 0){
      setTurnLength(tl);
    }

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

ResourceManager* Game::getResourceManager() const{
    return resourcemanager;
}

PlayerManager* Game::getPlayerManager() const{
    return playermanager;
}

void Game::setTurnProcess(TurnProcess* tp){
  if(turnprocess != NULL)
    delete turnprocess;
  turnprocess = tp;
}

TurnProcess* Game::getTurnProcess() const{
  return turnprocess;
}

DesignStore* Game::getDesignStore() const{
  return designstore;
}

Persistence* Game::getPersistence() const{
    return persistence;
}

bool Game::setPersistence(Persistence* p){
  if(!loaded){
    delete persistence;
    persistence = p;
    return true;
  }else{
    Logger::getLogger()->warning("Could not set new Persistence method.");
    return false;
  }
}

TpScheme* Game::getTpScheme() const{
  return tpscheme;
}

/*!Sets the implementation of TpScheme to use.

  If there is an implementation set already, it is not replaced and
  false is returned.

  \param imp The TpScheme implementation to use
  \returns True if set, false if not set
    
 */
bool Game::setTpScheme(TpScheme* imp){
  if(tpscheme != NULL)
    return false;
  tpscheme = imp;
  return true;
}

Random* Game::getRandom() const{
  return random;
}

bool Game::isLoaded() const{
  return loaded;
}

bool Game::isStarted() const{
  return (loaded && started);
}

void Game::doEndOfTurn(){
  if(loaded && started){
    Logger::getLogger()->info("End Of Turn started");

    // send frame to all connections that the end of turn has started
    Frame * frame = new Frame(fv0_2);
    frame->setType(ft02_Time_Remaining);
    frame->packInt(0);
    Network::getNetwork()->sendToAll(frame);

    // DO END OF TURN STUFF HERE
    turnprocess->doTurn();

    // increment the turn counter
    turnNum++;
    // save game info
    persistence->saveGameInfo();
    // increment the time to the next turn
    turnTime += turnIncrement;
    timer->setValid(false);
    delete timer;
    timer = NULL;
    if(secondsToEOT() <= 0)
      resetEOTTimer();
    else
      setEOTTimer();

    // send frame to all connections that the end of turn has finished
    frame = new Frame(fv0_2);
    frame->setType(ft02_Time_Remaining);
    frame->packInt(secondsToEOT());
    Network::getNetwork()->sendToAll(frame);
    Network::getNetwork()->doneEOT();

    Logger::getLogger()->info("End Of Turn finished");
  }else{
    Logger::getLogger()->info("End Of Turn not run because game not started");
    turnTime += turnIncrement;
    setEOTTimer();
  }
}

void Game::resetEOTTimer(){
  turnTime = time(NULL) + turnIncrement;
  setEOTTimer();
}

int Game::secondsToEOT(){
  return turnTime - time(NULL);
}

uint32_t Game::getTurnNumber() const{
  return turnNum;
}

void Game::setTurnLength(unsigned int sec){
    if(sec == 0){
        Logger::getLogger()->warning("Tried to set turn length to zero seconds, setting to 1 minute instead.");
        sec = 60;
    }
  turnIncrement = sec;
}

uint32_t Game::getTurnLength(){
  return turnIncrement;
}

void Game::saveAndClose()
{
	save();
	//remove and delete players

	//remove and delete objects
        
        //shutdown the persistence method.
        if(loaded){
          persistence->shutdown();
        }
        
	Logger::getLogger()->info("Game saved & closed");
}

void Game::packGameInfoFrame(Frame* frame){
  frame->setType(ft04_GameInfo);
  Settings* settings = Settings::getSettings();
  Advertiser* advertiser = Network::getNetwork()->getAdvertiser();
  if(settings->get("game_shortname").empty()){
    frame->packString("tp");
  }else{
    frame->packString(settings->get("game_shortname"));
  }
  frame->packString("");
  frame->packInt(2);
  frame->packString("0.3");
  frame->packString("0.4");
  frame->packString(VERSION);
  frame->packString("tpserver-cpp");
  if(ruleset != NULL){
    frame->packString(ruleset->getName());
    frame->packString(ruleset->getVersion());
  }else{
    frame->packString("");
    frame->packString("");
  }
  std::map<std::string, uint16_t> services = advertiser->getServices();
  frame->packInt(services.size());
  for(std::map<std::string, uint16_t>::iterator itcurr = services.begin();
      itcurr != services.end(); ++itcurr){
    frame->packString(itcurr->first);
    if(settings->get("metaserver_fake_dns") != ""){
      frame->packString(settings->get("metaserver_fake_dns"));
    }else{
      frame->packString(""); //TODO get dns automatically
    }
    if(settings->get("metaserver_fake_ip") != ""){
      frame->packString(settings->get("metaserver_fake_ip"));
    }else{
      frame->packString(""); //TODO get ip address automatically
    }
    frame->packInt(itcurr->second);
  }
  std::map<uint32_t, std::pair<std::string, uint32_t> > optionalparams;
  if(!settings->get("admin_email").empty()){
    optionalparams[4] = std::pair<std::string, uint32_t>(settings->get("admin_email"), 0);
  }
  if(!settings->get("game_comment").empty()){
    optionalparams[5] = std::pair<std::string, uint32_t>(settings->get("game_comment"), 0);
  }
  //number of optional params
  frame->packInt(7 + optionalparams.size());
  
  frame->packInt(8);
  if(settings->get("game_shortname").empty()){
    frame->packString("tp");
  }else{
    frame->packString(settings->get("game_shortname"));
  }
  frame->packInt(0);
  
  frame->packInt(7);
  if(settings->get("game_longname").empty()){
    frame->packString("Tpserver-cpp");
  }else{
    frame->packString(settings->get("game_longname"));
  }
  frame->packInt(0);
  
  frame->packInt(1);
  frame->packString("");
  frame->packInt(playermanager->getNumPlayers());
  
  frame->packInt(3);
  frame->packString("");
  frame->packInt(objectmanager->getNumObjects());
  
  frame->packInt(6);
  frame->packString("");
  frame->packInt(turnTime);
  
  frame->packInt(9);
  frame->packString("");
  frame->packInt(turnNum);
  
  frame->packInt(10);
  frame->packString("");
  frame->packInt(turnIncrement);
  
  for(std::map<uint32_t, std::pair<std::string, uint32_t> >::iterator itcurr = optionalparams.begin();
      itcurr != optionalparams.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packString(itcurr->second.first);
    frame->packInt(itcurr->second.second);
  }
  
  if(!settings->get("game_media_base").empty()){
    frame->packString(settings->get("game_media_base"));
  }else{
    frame->packString("http://darcs.thousandparsec.net/repos/media/client/");
  }
  
}

void Game::setTurnNumber(uint32_t t){
  turnNum = t;
}

void Game::setGameStartTime(uint64_t t){
  ctime = t;
}

void Game::setKey(const std::string& nk){
  key = nk;
}

uint64_t Game::getGameStartTime() const{
  return ctime;
}

std::string Game::getKey() const{
  return key;
}

Game::Game() : ctime(0), turnNum(0), key(){
  objectmanager = new ObjectManager();
  ordermanager = new OrderManager();
  objectdatamanager = new ObjectDataManager();
  boardmanager = new BoardManager();
  resourcemanager = new ResourceManager();
  playermanager = new PlayerManager();
  designstore = new DesignStore();
  turnprocess = NULL;
  ruleset = NULL;
  persistence = new Persistence();
  random = new Random();
  tpscheme = NULL;
  loaded = false;
  started = false;

  turnIncrement = 86400; //24 hours
  timer = NULL;
  resetEOTTimer();
  //this is a good place to seed the PNRG
  random->seed(getpid() + time(NULL));
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
    delete resourcemanager;
    delete playermanager;
  if(turnprocess != NULL)
    delete turnprocess;
  if(ruleset != NULL)
    delete ruleset;
  delete designstore;
    if(persistence != NULL)
        delete persistence;
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
  delete random;
}

Game Game::operator=(Game & rhs)
{
  //only here to stop people doing funny things...
  assert(0);
  return *this;
}

void Game::setEOTTimer(){
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
  timer = new TimerCallback(this, &Game::doEndOfTurn, turnTime - time(NULL));
  Network::getNetwork()->addTimer(*timer);
}
