#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings {

      public:
	static Settings *getSettings();

	bool readArgs(int argc, char** argv);

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
	 Settings(Settings & rhs);
	Settings operator=(Settings & rhs);

	void setDefaultValues();

	// settings storage

	static Settings *myInstance;

};

#endif
