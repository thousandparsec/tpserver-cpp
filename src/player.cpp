#include "string.h"

#include "connection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"
#include "order.h"

#include "player.h"

int Player::nextpid = 1;

Player::Player()
{
	curConnection = NULL;
	name = NULL;
	passwd = NULL;
	pid = nextpid++;
}

Player::~Player()
{
	if (name != NULL) {
		delete[]name;
	}
	if (passwd != NULL) {
		delete[]passwd;
	}
	if (curConnection != NULL) {
		curConnection->close();
	}
}

void Player::setName(char *newname)
{
	if (name != NULL) {
		delete[]name;
	}
	int len = strlen(newname) + 1;
	name = new char[len];
	strncpy(name, newname, len);
}

void Player::setPass(char *newpass)
{
	if (passwd != NULL) {
		delete[]passwd;
	}
	int len = strlen(newpass) + 1;
	passwd = new char[len];
	strncpy(passwd, newpass, len);
}

void Player::setConnection(Connection * newcon)
{
	curConnection = newcon;
}

void Player::setID(int newid)
{
	pid = newid;
}

char *Player::getName()
{
	int len = strlen(name) + 1;
	char *temp = new char[len];
	strncpy(temp, name, len);
	return temp;
}

char *Player::getPass()
{
	int len = strlen(passwd) + 1;
	char *temp = new char[len];
	strncpy(temp, passwd, len);
	return temp;
}

Connection *Player::getConnection()
{
	return curConnection;
}

int Player::getID()
{
	return pid;
}

void Player::processIGFrame(Frame * frame)
{
  Logger::getLogger()->debug("IG Frame processor");
  
	switch (frame->getType()) {
	case ft02_Object_GetById:
		processGetObjectById(frame);
		break;
	case ft02_Object_GetByPos:
	  processGetObjectByPos(frame);
	  break;
	case ft02_Order_Get:
		processGetOrder(frame);
		break;
	case ft02_Order_Add:
		processAddOrder(frame);
		break;
	case ft02_Order_Remove:
		processRemoveOrder(frame);
		break;
	case ft02_OrderDesc_Get:
		processDescribeOrder(frame);
		break;
		//more
	default:
		Logger::getLogger()->warning("Player: Discarded frame, not processed");
		break;
	}

  

	delete frame;
}



void Player::processGetObjectById(Frame * frame)
{
	Logger::getLogger()->debug("doing get object by id frame");

	Frame *of = curConnection->createFrame(frame);
	if (frame->getDataLength() >= 4) {
		unsigned int len = frame->unpackInt();

		// Check we have enough data
		if (frame->getDataLength() >= 4 + 4*len) {
			
			// ListSeq Frame
	    	of->setType(ft02_Sequence);
    		of->packInt(len);
	    	curConnection->sendFrame(of);

			// Object frames
    		for(int i=0 ; i < len; ++i) {
		  unsigned int objectID = frame->unpackInt();
		  
		  of = curConnection->createFrame(frame);
		  
		  IGObject* o = Game::getGame()->getObject(objectID);
		  if (o != NULL) {
		    o->createFrame(of, pid);
		  } else {
		    of->createFailFrame(fec_NonExistant, "No such object");
		  }
		  curConnection->sendFrame(of);
		}
		
		return;
		}
	}

	// Fall through incase of error
	of->createFailFrame(fec_FrameError, "Invalid frame");
	curConnection->sendFrame(of);
}

void Player::processGetObjectByPos(Frame * frame)
{
  Logger::getLogger()->debug("doing get object by pos frame");
  Frame *of = curConnection->createFrame(frame);
  if (frame->getDataLength() >= 36) {
    long long x, y, z;
    unsigned long long r;

    x = frame->unpackInt64();
    y = frame->unpackInt64();
    z = frame->unpackInt64();
    r = frame->unpackInt64();

    std::list<unsigned int> oblist = Game::getGame()->getObjectsByPos(x, y, z, r);

    of->setType(ft02_Sequence);
    of->packInt(oblist.size());
    curConnection->sendFrame(of);

    std::list<unsigned int>::iterator obCurr = oblist.begin();
    for( ; obCurr != oblist.end(); ++obCurr) {
      of = curConnection->createFrame(frame);
      Game::getGame()->getObject(*obCurr)->createFrame(of, pid);
      curConnection->sendFrame(of);
    }

  } else {
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
  }
}



void Player::processGetOrder(Frame * frame)
{
  Logger::getLogger()->debug("Doing get order frame");
  
  if(frame->getDataLength() < 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  int objectID = frame->unpackInt();
  int num_orders = frame->unpackInt();

  if(frame->getDataLength() != 8 + 4 * num_orders){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  if(num_orders > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(num_orders);
    curConnection->sendFrame(seq);
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);
    int ordpos = frame->unpackInt();
    Order *ord = Game::getGame()->getObject(objectID)->getOrder(ordpos, pid);
    if (ord != NULL) {
      ord->createFrame(of, objectID, ordpos);
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not get Order");
    }
        
    curConnection->sendFrame(of);
  }
  

}


void Player::processAddOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing add order frame");
	Frame *of = curConnection->createFrame(frame);
	if (frame->getDataLength() >= 8) {
		unsigned int objectID = frame->unpackInt();
		Order *ord = Order::createOrder((OrderType) frame->unpackInt());
		int pos = frame->unpackInt();
		ord->inputFrame(frame);
		if (Game::getGame()->getObject(objectID)->addOrder(ord, pos, pid)) {
		  
		    of->setType(ft02_OK);
			of->packString("Order Added");
		} else {
			of->createFailFrame(fec_TempUnavailable, "Could not add order");
		}
	} else {
		of->createFailFrame(fec_FrameError, "Invalid frame");
	}
	curConnection->sendFrame(of);
}


void Player::processRemoveOrder(Frame * frame)
{
  Logger::getLogger()->debug("doing remove order frame");

  if(frame->getDataLength() < 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  int objectID = frame->unpackInt();
  int num_orders = frame->unpackInt();

  if(frame->getDataLength() != 8 + 4 * num_orders){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  if(num_orders > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(num_orders);
    curConnection->sendFrame(seq);
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);
    int ordpos = frame->unpackInt();
    if (Game::getGame()->getObject(objectID)->removeOrder(ordpos, pid)) {
      of->setType(ft02_OK);
      of->packString("Order removed");
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not remove Order");
    }

    curConnection->sendFrame(of);
    
  }

}


void Player::processDescribeOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing describe order frame");
	Frame *of = curConnection->createFrame(frame);
	int ordertype = frame->unpackInt();
	Order::describeOrder(ordertype, of);
	curConnection->sendFrame(of);
}

