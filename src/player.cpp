/*  Player object, processing and frame handling
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include "string.h"

#include "connection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"
#include "order.h"
#include "vector3d.h"
#include "board.h"
#include "message.h"
#include "ordermanager.h"
#include "objectdata.h"

#include "player.h"

int Player::nextpid = 1;

Player::Player()
{
	curConnection = NULL;
	name = NULL;
	passwd = NULL;
	pid = nextpid++;
	board = new Board();
	board->setBoardID(0);
	board->setName("Personal board");
	board->setDescription("System board");

	Message * msg = new Message();
	msg->setType(0);
	msg->setSubject("Welcome");
	msg->setBody("Welcome to Thousand Parsec!\nThis server is running on tpserver-cpp.  Please report any problems and enjoy the game.");
	board->addMessage(msg, -1);
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
	delete board;
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

void Player::postToBoard(Message* msg){
  board->addMessage(msg, -1);
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
	case ft02_Order_Insert:
		processAddOrder(frame);
		break;
	case ft02_Order_Remove:
		processRemoveOrder(frame);
		break;
	case ft02_OrderDesc_Get:
		processDescribeOrder(frame);
		break;
	case ft02_Time_Remaining_Get:
		processGetTime(frame);
		break;
	case ft02_Board_Get:
	  processGetBoards(frame);
	  break;
	case ft02_Message_Get:
	  processGetMessages(frame);
	  break;
	case ft02_Message_Post:
	  processPostMessage(frame);
	  break;
	case ft02_Message_Remove:
	  processRemoveMessages(frame);
	  break;
	case ft03_Order_Probe:
	  processProbeOrder(frame);
	  break;
	default:
		Logger::getLogger()->warning("Player: Discarded frame, not processed");

		// Send a failed frame
  		Frame *of = curConnection->createFrame(frame);
		of->createFailFrame(fec_ProtocolError, "Did not understand that frame type.");
		curConnection->sendFrame(of);
		break;
	}

	delete frame;
}



void Player::processGetObjectById(Frame * frame)
{
	Logger::getLogger()->debug("doing get object by id frame");

	Frame *of = curConnection->createFrame(frame);
	if (frame->getDataLength() >= 4) {
		int len = frame->unpackInt();

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
    Vector3d pos;
    unsigned long long r;

    pos.unpack(frame);
    r = frame->unpackInt64();

    std::list<unsigned int> oblist = Game::getGame()->getObjectsByPos(pos, r);

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
  IGObject *o = Game::getGame()->getObject(objectID);
  if (o == NULL) {
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_NonExistant, "No such object");
    curConnection->sendFrame(of);
    return;
  }
  
  int num_orders = frame->unpackInt();
  if(frame->getDataLength() != 8 + 4 * num_orders){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  if(num_orders > 1) {
  	Logger::getLogger()->debug("Got multiple orders, returning a sequence");
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(num_orders);
    curConnection->sendFrame(seq);
  } else {
  	Logger::getLogger()->debug("Got single orders, returning one object");
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);

    int ordpos = frame->unpackInt();
    Order *ord = o->getOrder(ordpos, pid);
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

		// See if we have a valid object number
		unsigned int objectID = frame->unpackInt();
		IGObject *o = Game::getGame()->getObject(objectID);
		if (o == NULL) {
			of->createFailFrame(fec_NonExistant, "No such object");
			
		} else {
			// Order Slot
			int pos = frame->unpackInt();

			// See if we have a valid order
			Order *ord = Game::getGame()->getOrderManager()->createOrder(frame->unpackInt());
			if (ord == NULL) {
				of->createFailFrame(fec_NonExistant, "No such order type");
			} else {
				
				if (ord->inputFrame(frame) && o->addOrder(ord, pos, pid)) {
				    of->setType(ft02_OK);
					of->packString("Order Added");
				} else {
					of->createFailFrame(fec_TempUnavailable, "Could not add order");
				}
			}
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
    IGObject * obj = Game::getGame()->getObject(objectID);
    if (obj != NULL && obj->removeOrder(ordpos, pid)) {
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

	int numdesc = frame->unpackInt();
	if(numdesc > 1){
	  Frame *seq = curConnection->createFrame(frame);
	  seq->setType(ft02_Sequence);
	  seq->packInt(numdesc);
	  curConnection->sendFrame(seq);
	}
	
	if(numdesc == 0){
	  Frame *of = curConnection->createFrame(frame);
	  Logger::getLogger()->debug("asked for no orders to describe, silly client...");
	  of->createFailFrame(fec_NonExistant, "You didn't ask for any order descriptions, try again");
	  curConnection->sendFrame(of);
	}

	for(int i = 0; i < numdesc; i++){
	  Frame *of = curConnection->createFrame(frame);
	  int ordertype = frame->unpackInt();
	  Game::getGame()->getOrderManager()->describeOrder(ordertype, of);
	  curConnection->sendFrame(of);
	}
}

void Player::processProbeOrder(Frame * frame){
  Logger::getLogger()->debug("doing probe order frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Probe order isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }

  int obid = frame->unpackInt();
  IGObject* theobject = Game::getGame()->getObject(obid);
  if(theobject == NULL){
    Logger::getLogger()->debug("The object the probed orders are for doesn't exist");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_NonExistant, "No such object");
    curConnection->sendFrame(of);
  }

  int pos = frame->unpackInt();
  
  Frame *of = curConnection->createFrame(frame);
  // See if we have a valid order
  Order *ord = Game::getGame()->getOrderManager()->createOrder(frame->unpackInt());
  if (ord == NULL) {
    of->createFailFrame(fec_NonExistant, "No such order type");
  }else if(theobject->getObjectData()->checkAllowedOrder(ord->getType(), pid)){
    
    ord->inputFrame(frame);
    ord->createFrame(of, obid, pos);
    
  }else{
    
    Logger::getLogger()->debug("The order to be probed is not allowed on this object");
    of->createFailFrame(fec_PermUnavailable, "The order to be probed is not allowed on this object, try again");
    
  }
  curConnection->sendFrame(of);
  
  
}

void Player::processGetTime(Frame * frame){
  Logger::getLogger()->debug("doing Get Time frame");
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft02_Time_Remaining);
  of->packInt(Game::getGame()->secondsToEOT());
  curConnection->sendFrame(of);
}

void Player::processGetBoards(Frame * frame){
  Logger::getLogger()->debug("doing Get Boards frame");
  
  int numboards = frame->unpackInt();
  if(numboards > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numboards);
    curConnection->sendFrame(seq);
  }
  
  if(numboards == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no boards, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any boards, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numboards; i++){
    Frame *of = curConnection->createFrame(frame);
    int boardnum = frame->unpackInt();
    if(boardnum == 0){
      board->packBoard(of);
    }else{
      //boards in the game object
      of->createFailFrame(fec_PermUnavailable, "No non-player boards yet");
    }
    curConnection->sendFrame(of);
  }
}

void Player::processGetMessages(Frame * frame){
  Logger::getLogger()->debug("doing Get Messages frame");

  int boardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  if(nummsg > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(nummsg);
    curConnection->sendFrame(seq);
  }

  if(nummsg == 0){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "No messages to get");
    curConnection->sendFrame(of);
    
    return;
  }

  Board * currboard;
  if(boardid == 0){
    currboard = board;
  }else{
    //board in game object
    currboard = NULL;
  }

  if(board != NULL){
    for(int i = 0; i < nummsg; i++){
      Frame *of = curConnection->createFrame(frame);
      int msgnum = frame->unpackInt();

      board->packMessage(of, msgnum);

      curConnection->sendFrame(of);
    
    }

  }else{
     Frame *of = curConnection->createFrame(frame);
     of->createFailFrame(fec_NonExistant, "Board does not exist");
     curConnection->sendFrame(of);
  }

}

void Player::processPostMessage(Frame * frame){
  Logger::getLogger()->debug("doing Post Messages frame");

  Frame *of = curConnection->createFrame(frame);

  int boardid = frame->unpackInt();
  int pos = frame->unpackInt();

  Board * currboard;
  if(boardid == 0){
    currboard = board;
  }else{
    //board in game object
    currboard = NULL;
  }

  if(currboard != NULL){
    Message* msg = new Message();
    frame->unpackInt(); //list header, is hopefully 1
    msg->setType(frame->unpackInt());
    msg->setSubject(std::string(frame->unpackString()));
    msg->setBody(std::string(frame->unpackString()));

    currboard->addMessage(msg, pos);

    of->setType(ft02_OK);
    of->packString("Message posted");
    
  }else{
    of->createFailFrame(fec_NonExistant, "Board does not exist");
  }

  curConnection->sendFrame(of);
  
}

void Player::processRemoveMessages(Frame * frame){
  Logger::getLogger()->debug("doing Remove Messages frame");

  int boardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  if(nummsg > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(nummsg);
    curConnection->sendFrame(seq);
  }

  Board * currboard;
  if(boardid == 0){
    currboard = board;
  }else{
    //board in game object
    currboard = NULL;
  }

  if(board != NULL){
    for(int i = 0; i < nummsg; i++){
      Frame *of = curConnection->createFrame(frame);
      int msgnum = frame->unpackInt();

      if(board->removeMessage(msgnum)){
	of->setType(ft02_OK);
	of->packString("Message removed");
      }else{
	of->createFailFrame(fec_NonExistant, "Message not removed, does exist");
      }

      curConnection->sendFrame(of);
    
    }

  }else{
     Frame *of = curConnection->createFrame(frame);
     of->createFailFrame(fec_NonExistant, "Board does not exist");
     curConnection->sendFrame(of);
  }

}
