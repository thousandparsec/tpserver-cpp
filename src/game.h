#ifndef GAME_H
#define GAME_H

#include <list>

class Player;

class Game{

 public:
  static Game* getGame();

  void createRandomUniverse();
  void createRealUniverse();
  void createTutorial();
  //void loadGame(char* file);
  //void setSaveFile(char *file);
  void save();

  Player* findPlayer(char* name, char* pass);

  void doEndOfTurn();

  void saveAndClose();


 private:
  Game();
  Game(Game &rhs);
  ~Game();
  Game operator=(Game &rhs);

  static Game *myInstance;

  std::list<Player*> players;

};

#endif
