#ifndef OBJECTDATA_H
#define OBJECTDATA_H


class Game;
class Frame;
class IGObject;

class ObjectData {

      public:

	virtual void packExtraData(Frame * frame) = 0;

	virtual void doOnceATurn(IGObject * obj) = 0;

      protected:


      private:


};

#endif
