#ifndef GAME_H
#define GAME_H

#include <list>
#include <map>

class Player;
class IGObject;
class Vector3d;
class OrderManager;
class ObjectDataManager;

class Game {

      public:
	static Game *getGame();

	void createRandomUniverse();
	void createRealUniverse();
	void createTutorial();
	//void loadGame(char *file);
	//void setSaveFile(char *file);
	void save();

	Player *findPlayer(char *name, char *pass);

	IGObject *getObject(unsigned int id);
	void addObject(IGObject* obj);

	std::list<unsigned int> getObjectsByPos(const Vector3d & pos, unsigned long long r);
	std::list<unsigned int> getContainerByPos(const Vector3d & pos);

	OrderManager* getOrderManager() const;
	ObjectDataManager* getObjectDataManager() const;

	void doEndOfTurn();
	void resetEOTTimer();

	int secondsToEOT();

	void saveAndClose();


      private:
	Game();
	Game(Game & rhs);
	~Game();
	Game operator=(Game & rhs);
	
	static Game *myInstance;
	
	int turnTime;
	int turnIncrement;
	
	std::list<Player *> players;
	
	std::map<unsigned int, IGObject *> objects;
	IGObject *universe;

	OrderManager * ordermanager;
	ObjectDataManager * objectdatamanager;

};

#endif
