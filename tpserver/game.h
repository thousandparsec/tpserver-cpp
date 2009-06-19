#ifndef GAME_H
#define GAME_H
/*  Game class
 *
 *  Copyright (C) 2004-2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <list>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <exception>

class Player;
class IGObject;
class Vector3d;
class ObjectManager;
class OrderManager;
class ObjectTypeManager;
class BoardManager;
class ResourceManager;
class PlayerManager;
class TurnProcess;
class DesignStore;
class Ruleset;
class Persistence;
class TpScheme;
class TimerCallback;
class Random;
class Frame;
class TurnTimer;

class Game {

  public:
    static Game *getGame();

    bool setRuleset(Ruleset* rs);
    Ruleset* getRuleset() const;

    bool load();
    bool start();

    void save();

    ObjectManager* getObjectManager() const;
    OrderManager* getOrderManager() const;
    ObjectTypeManager* getObjectTypeManager() const;
    BoardManager* getBoardManager() const;
    ResourceManager* getResourceManager() const;
    PlayerManager* getPlayerManager() const;
    
    void setTurnProcess(TurnProcess* tp);
    TurnProcess* getTurnProcess() const;

    DesignStore* getDesignStore() const;

    Persistence* getPersistence() const;
    bool setPersistence(Persistence* p);

    bool setTpScheme(TpScheme* imp);
    TpScheme* getTpScheme() const;
    
    Random* getRandom() const;
    
    bool setTurnTimer(TurnTimer* tt);
    TurnTimer* getTurnTimer() const;
  
    bool isLoaded() const;
    bool isStarted() const;

    // only to be called by TurnTimer and subclasses
    void doEndOfTurn();

    uint32_t getTurnNumber() const;
    std::string getTurnName() const;
    void setTurnName(const std::string& tn);

    uint64_t getGameStartTime() const;

    void saveAndClose();

    void packGameInfoFrame(Frame* frame);
    
    //For persistence only
    void setTurnNumber(uint32_t t);
    void setGameStartTime(uint64_t t);
    void setKey(const std::string& nk);

    //Persistence and Advertiser/publishers only
    std::string getKey() const;

  private:
    Game();
    Game(Game & rhs);
    ~Game();
    Game operator=(Game & rhs);
    
    static Game *myInstance;
    
    bool loaded;
    bool started;
    
    uint64_t ctime;
    uint32_t turnNum;
    std::string turnname;
    std::string key;
    
    Ruleset* ruleset;

    ObjectManager* objectmanager;
    OrderManager * ordermanager;
    ObjectTypeManager * objecttypemanager;
    BoardManager* boardmanager;
    ResourceManager* resourcemanager;
    PlayerManager* playermanager;
    TurnProcess * turnprocess;
    DesignStore* designstore;
    Persistence* persistence;
    TpScheme* tpscheme;
    Random* random;
    TurnTimer* turntimer;

};

/** Exception for when Ruleset::createGame fails.

For example, in an implementation of createGame:

if (failed) {
    throw GameCreateFailed("Just can't perform createGame.");
}
 */
class GameCreateFailed : public std::exception{
    public:
        /** Construct the GameCreateFailed exception.
        @param reason The reason for the exception.
         */
        GameCreateFailed(const std::string& reason);
        
        /** Destructor.
        */
        virtual ~GameCreateFailed() throw();
        
        /** Return the reason for the exception.
        @return String of the reason for the exception.
         */
        virtual const char* what() const throw();
    private:
        std::string why;
};

#endif
