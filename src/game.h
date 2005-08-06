#ifndef GAME_H
#define GAME_H
/*  Game class
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

class Player;
class IGObject;
class Vector3d;
class OrderManager;
class ObjectDataManager;
class CombatStrategy;
class DesignStore;
class Ruleset;

class Game {

      public:
	static Game *getGame();

	bool setRuleset(Ruleset* rs);
	Ruleset* getRuleset() const;

	bool load();
	bool start();

	void save();

	Player *findPlayer(char *name, char *pass);
	Player* getPlayer(unsigned int id);
	std::set<unsigned int> getPlayerIds() const;

	IGObject *getObject(unsigned int id);
	void addObject(IGObject* obj);
	void scheduleRemoveObject(unsigned int id);

	std::list<unsigned int> getObjectsByPos(const Vector3d & pos, unsigned long long r);
	std::list<unsigned int> getContainerByPos(const Vector3d & pos);
	std::set<unsigned int> getObjectIds() const;

	OrderManager* getOrderManager() const;
	ObjectDataManager* getObjectDataManager() const;
	
	CombatStrategy* getCombatStrategy() const;
	void setCombatStrategy(CombatStrategy* cs);

	DesignStore* getDesignStore() const;

	bool isLoaded() const;
	bool isStarted() const;

	void doEndOfTurn();
	void resetEOTTimer();
	int getTurnNumber();

	int secondsToEOT();
	void setTurnLength(unsigned int sec);

	void saveAndClose();


      private:
	Game();
	Game(Game & rhs);
	~Game();
	Game operator=(Game & rhs);
	
	static Game *myInstance;
	
	int turnTime;
	int turnIncrement;

	bool loaded;
	bool started;
	
	std::map<unsigned int, Player *> players;
	
	std::map<unsigned int, IGObject *> objects;
	IGObject *universe;

	std::set<unsigned int> scheduleRemove;

	Ruleset* ruleset;

	OrderManager * ordermanager;
	ObjectDataManager * objectdatamanager;
	CombatStrategy * combatstrategy;
	DesignStore* designstore;

};

#endif
