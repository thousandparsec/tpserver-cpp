#ifndef GAME_H
#define GAME_H

#include <list>
#include <map>

class Player;
class IGObject;

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

	std::list <unsigned int> getObjectsByPos(long long x, long long y, long long z, unsigned long long r);
	std::list <unsigned int> getContainerByPos(long long x, long long y, long long z);

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

	 std::list < Player * >players;

	 std::map < unsigned int, IGObject * >objects;
	IGObject *universe;

};

#endif
