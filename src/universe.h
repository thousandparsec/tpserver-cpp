#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "objectdata.h"

class Universe:public ObjectData {
      public:
	Universe();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	ObjectData* clone();

	void setYear(int year);
	int getYear();

      private:
	int yearNum;

};

#endif
