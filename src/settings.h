#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings{

 public:
  static Settings* getSettings();

  /*
  void set(int item, int value);
  void set(int item, std::string value);
  void set(int item, bool value);

  int get(int item);
  std::string get(int item);
  bool get(int item);
  */


 private:
  Settings();
  ~Settings();
  Settings(Settings &rhs);
  Settings operator=(Settings &rhs);

  // settings storage

  static Settings* myInstance;

};

#endif
