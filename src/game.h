#ifndef GAME_H
#define GAME_H

class Game{

 public:
  static Game* getGame();

  void createRandomUniverse();
  void createRealUniverse();
  void createTutorial();
  //void loadGame(char* file);
  //void setSaveFile(char *file);
  void save();

  void doEndOfTurn();

  void saveAndClose();


 private:
  Game();
  Game(Game &rhs);
  ~Game();
  Game operator=(Game &rhs);

  static Game *myInstance;

};

#endif
