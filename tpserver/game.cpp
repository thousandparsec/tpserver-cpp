/*  Game controller for tpserver-cpp
 *
 *  Copyright (C) 2003-2006, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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
#include "playerview.h"
#include "object.h"
#include "order.h"
#include "net.h"
#include "vector3d.h"
#include "objectmanager.h"
#include "ordermanager.h"
#include "objecttypemanager.h"
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
#include "turntimer.h"

#include "basicturntimer.h"
#include "playersfinishedturntimer.h"
#include "thresholdturntimer.h"

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
      if(Settings::getSettings()->get("metaserver_key_unsafe") == ""){
        std::ostringstream formater;
        formater << std::ios::hex << random->getInt32() << std::ios::hex << random->getInt32();
        key = formater.str();
      }else{
        key = Settings::getSettings()->get("metaserver_key_unsafe");
      }
      persistence->saveGameInfo();
      try{
        ruleset->createGame();
      }catch(GameCreateFailed& excep){
          Logger::getLogger()->error("Failed to create game, ruleset gave reason \"%s\"", excep.what());
          return false;
      }
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

    if(turntimer == NULL){
        std::string timername = Settings::getSettings()->get("turntimer");
        if(timername == "threshold"){
            turntimer = new ThresholdTurnTimer();
        }else if(timername == "playersfinished"){
            turntimer = new PlayersFinishedTurnTimer();
        }else{
            turntimer = new BasicTurnTimer();
        }
    }
    
    //set num of dead players in the TurnTimer for accurate threshold calcuation
    IdSet players = playermanager->getAllIds();
    uint32_t numdeadplayers = 0;
    for(IdSet::iterator itcurr = players.begin();
        itcurr != players.end(); ++itcurr){
      if (!playermanager->getPlayer(*itcurr)->isAlive() )  {
        numdeadplayers++;
      }
    }
    
    turntimer->setNumberDeadPlayers(numdeadplayers);
    
    turntimer->resetTimer();

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

ObjectTypeManager* Game::getObjectTypeManager() const{
  return objecttypemanager;
}

BoardManager* Game::getBoardManager() const{
    return boardmanager;
}

ResourceManager::Ptr Game::getResourceManager() const{
    return resourcemanager;
}

PlayerManager::Ptr Game::getPlayerManager() const{
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

DesignStore::Ptr Game::getDesignStore() const{
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

bool Game::setTurnTimer(TurnTimer* tt){
  if(!started){
    if(turntimer != NULL)
      delete turntimer;
    turntimer = tt;
    return true;
  }else
    return false;
}

TurnTimer* Game::getTurnTimer() const{
  return turntimer;
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

    // increment the turn counter
    turnNum++;

    // DO END OF TURN STUFF HERE
    turnprocess->doTurn();

    IdSet players = playermanager->getAllIds();
    uint32_t numdeadplayers = 0;
    for(IdSet::iterator itcurr = players.begin();
        itcurr != players.end(); ++itcurr){
      Player::Ptr player = playermanager->getPlayer(*itcurr);
      player->getPlayerView()->doOnceATurn();
      if(!player->isAlive()){
        numdeadplayers++;
      }
    }
    
    turntimer->setNumberDeadPlayers(numdeadplayers);

    // save game info
    persistence->saveGameInfo();

    Logger::getLogger()->info("End Of Turn finished");
  }else{
    Logger::getLogger()->info("End Of Turn not run because game not started");
  }
}

uint32_t Game::getTurnNumber() const{
  return turnNum;
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

void Game::packGameInfoFrame(OutputFrame::Ptr frame){
  frame->setType(ft04_GameInfo);
  Settings* settings = Settings::getSettings();
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

  Advertiser::ServiceMap services = Network::getNetwork()->getAdvertiser()->getServices();
  frame->packInt(services.size());
  for(Advertiser::ServiceMap::iterator itcurr = services.begin();
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
  if(turntimer != NULL){
    optionalparams[6] = std::pair<std::string, uint32_t>("", turntimer->secondsToEOT() + time(NULL));
    optionalparams[10] = std::pair<std::string, uint32_t>("", turntimer->getTurnLength());
  }
  //number of optional params
  frame->packInt(5 + optionalparams.size());
  
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
  
  frame->packInt(9);
  frame->packString("");
  frame->packInt(turnNum);
  
  for(std::map<uint32_t, std::pair<std::string, uint32_t> >::iterator itcurr = optionalparams.begin();
      itcurr != optionalparams.end(); ++itcurr){
    frame->packInt(itcurr->first);
    frame->packString(itcurr->second.first);
    frame->packInt(itcurr->second.second);
  }
  
  if(!settings->get("game_media_base").empty()){
    frame->packString(settings->get("game_media_base"));
  }else{
    frame->packString("http://svn.thousandparsec.net/svn/media/client/");
  }
  frame->packInt64(ctime);
}

void Game::setTurnNumber(uint32_t t){
  turnNum = t;
}

std::string Game::getTurnName() const{
    return turnname;
}

void Game::setTurnName(const std::string& tn){
    turnname = tn;
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

Game::Game() : ctime(0), turnNum(0),turnname(""), key(), turntimer(NULL){
  objectmanager = new ObjectManager();
  ordermanager = new OrderManager();
  objecttypemanager = new ObjectTypeManager();
  boardmanager = new BoardManager();
  resourcemanager.reset( new ResourceManager() );
  playermanager.reset( new PlayerManager() );
  designstore.reset( new DesignStore() );
  turnprocess = NULL;
  ruleset = NULL;
  persistence = new Persistence();
  random = new Random();
  tpscheme = NULL;
  loaded = false;
  started = false;

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
  delete objecttypemanager;
    delete boardmanager;
  if(turnprocess != NULL)
    delete turnprocess;
  if(ruleset != NULL)
    delete ruleset;
    if(persistence != NULL)
        delete persistence;
  delete random;
  if(turntimer != NULL)
    delete turntimer;
}

Game Game::operator=(Game & rhs)
{
  //only here to stop people doing funny things...
  assert(0);
  return *this;
}

GameCreateFailed::GameCreateFailed(const std::string& reason) : std::exception(){
    why = reason;
}

GameCreateFailed::~GameCreateFailed() throw() {}

const char* GameCreateFailed::what() const throw(){
    return why.c_str();
}
