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

#include <algorithm>

// #include "string.h"

#include "playerconnection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"
#include "order.h"
#include "vector3d.h"
#include "board.h"
#include "message.h"
#include "objectmanager.h"
#include "ordermanager.h"
#include "boardmanager.h"
#include "playermanager.h"
#include "objectdata.h"
#include "designstore.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"

#include "player.h"


Player::Player()
{
	curConnection = NULL;
	pid = 0;
	currObjSeq = 0;
    boardid = 0;
}

Player::~Player()
{
	if (curConnection != NULL) {
		curConnection->close();
	}
}

void Player::setName(const std::string& newname)
{
    name = newname;
}

void Player::setPass(const std::string& newpass)
{
    passwd = newpass;
}

void Player::setConnection(PlayerConnection * newcon)
{
	curConnection = newcon;
}

void Player::setId(uint32_t newid)
{
	pid = newid;
}

void Player::setVisibleObjects(std::set<unsigned int> vis){
  visibleObjects = vis;
  currObjSeq++;
}

bool Player::isVisibleObject(unsigned int objid){
  return visibleObjects.find(objid) != visibleObjects.end();
}

std::set<uint32_t> Player::getVisibleObjects() const{
    return visibleObjects;
}

void Player::addVisibleDesign(unsigned int designid){
  visibleDesigns.insert(designid);
}

void Player::addUsableDesign(unsigned int designid){
  usableDesigns.insert(designid);
  Logger::getLogger()->debug("Added valid design");
}

void Player::removeUsableDesign(unsigned int designid){
  std::set<unsigned int>::iterator dicurr = usableDesigns.find(designid);
  if(dicurr != usableDesigns.end())
    usableDesigns.erase(dicurr);
}

bool Player::isUsableDesign(unsigned int designid) const{
  return (usableDesigns.find(designid) != usableDesigns.end());
}

std::set<unsigned int> Player::getUsableDesigns() const{
  return usableDesigns;
}

std::set<uint32_t> Player::getVisibleDesigns() const{
    return visibleDesigns;
}

void Player::addVisibleComponent(unsigned int compid){
  visibleComponents.insert(compid);
}

void Player::addUsableComponent(unsigned int compid){
  usableComponents.insert(compid);
}

void Player::removeUsableComponent(unsigned int compid){
  std::set<unsigned int>::iterator cicurr = usableComponents.find(compid);
  if(cicurr != usableComponents.end())
    usableComponents.erase(cicurr);
}

bool Player::isUsableComponent(unsigned int compid){
  return (usableComponents.find(compid) != usableComponents.end());
}

std::set<uint32_t> Player::getVisibleComponents() const{
    return visibleComponents;
}

std::set<uint32_t> Player::getUsableComponents() const{
    return usableComponents;
}

void Player::postToBoard(Message* msg){
    Board* board = Game::getGame()->getBoardManager()->getBoard(boardid);
    board->addMessage(msg, -1);
}

std::string Player::getName() const{
    return name;
}

std::string Player::getPass() const{
    return passwd;
}

PlayerConnection *Player::getConnection()
{
	return curConnection;
}

int Player::getID()
{
	return pid;
}

uint32_t Player::getBoardId() const{
    return boardid;
}

void Player::setBoardId(uint32_t nbi){
    boardid = nbi;
}

void Player::packFrame(Frame* frame){
    frame->setType(ft03_Player);
  frame->packInt(pid);
  frame->packString(name.c_str());
  frame->packString("Human");
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
	case ft02_OrderDesc_Get:
		processDescribeOrder(frame);
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
	case ft02_ResDesc_Get:
	  processGetResourceDescription(frame);
	  break;

	case ft03_ObjectIds_Get:
	  processGetObjectIds(frame);
	  break;
	case ft03_ObjectIds_GetByPos:
	  processGetObjectIdsByPos(frame);
	  break;
	case ft03_ObjectIds_GetByContainer:
	  processGetObjectIdsByContainer(frame);
	  break;
	case ft03_OrderTypes_Get:
	  processGetOrderTypes(frame);
	  break;
	case ft03_Order_Probe:
	  processProbeOrder(frame);
	  break;
	case ft03_BoardIds_Get:
	  processGetBoardIds(frame);
	  break;
	case ft03_ResType_Get:
	  processGetResourceTypes(frame);
	  break;
	case ft03_Player_Get:
	  processGetPlayer(frame);
	  break;

	case ft03_Category_Get:
	  processGetCategory(frame);
	  break;
	case ft03_Category_Add:
	case ft03_Category_Remove:
	  // don't allow either of these
	  processPermDisabled(frame);
	  break;
	case ft03_CategoryIds_Get:
	  processGetCategoryIds(frame);
	  break;
	case ft03_Design_Get:
	  processGetDesign(frame);
	  break;
	case ft03_Design_Add:
	  processAddDesign(frame);
	  break;
	case ft03_Design_Modify:
	  processModifyDesign(frame);
	  break;
	case ft03_Design_Remove:
	  //currently disabled
	  processPermDisabled(frame);
	  break;
	case ft03_DesignIds_Get:
	  processGetDesignIds(frame);
	  break;
	case ft03_Component_Get:
	  processGetComponent(frame);
	  break;
	case ft03_ComponentIds_Get:
	  processGetComponentIds(frame);
	  break;
	case ft03_Property_Get:
	  processGetProperty(frame);
	  break;
	case ft03_PropertyIds_Get:
	  processGetPropertyIds(frame);
	  break;

	default:
        Logger::getLogger()->warning("Player: Discarded frame, not processed, was type %d", frame->getType());

		// Send a failed frame
  		Frame *of = curConnection->createFrame(frame);
		of->createFailFrame(fec_ProtocolError, "Did not understand that frame type.");
		curConnection->sendFrame(of);
		break;
	}

	delete frame;
}



void Player::processPermDisabled(Frame * frame){
  Logger::getLogger()->debug("doing a frame that is disabled");

  Frame *of = curConnection->createFrame(frame);
  of->createFailFrame(fec_PermUnavailable, "Server does not support this frame type");
  curConnection->sendFrame(of);
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
		  
		  if(visibleObjects.find(objectID) != visibleObjects.end()){

                    IGObject* o = Game::getGame()->getObjectManager()->getObject(objectID);
		    if (o != NULL) {
		      o->createFrame(of, pid);
                        Game::getGame()->getObjectManager()->doneWithObject(objectID);
		    } else {
		      of->createFailFrame(fec_NonExistant, "No such object");
		    }
		  }else{
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

    std::set<unsigned int> oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);

    for(std::set<unsigned int>::iterator vischk = oblist.begin(); vischk != oblist.end();){
      if(visibleObjects.find(*vischk) == visibleObjects.end()){
	std::set<unsigned int>::iterator temp = vischk;
	++temp;
	oblist.erase(vischk);
	vischk = temp;
      }else{
	++vischk;
      }
    }

    of->setType(ft02_Sequence);
    of->packInt(oblist.size());
    curConnection->sendFrame(of);

    std::set<unsigned int>::iterator obCurr = oblist.begin();
    for( ; obCurr != oblist.end(); ++obCurr) {
      of = curConnection->createFrame(frame);
      Game::getGame()->getObjectManager()->getObject(*obCurr)->createFrame(of, pid);
      Game::getGame()->getObjectManager()->doneWithObject(*obCurr);
      curConnection->sendFrame(of);
    }

  } else {
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
  }
}

void Player::processGetObjectIds(Frame * frame){
  Logger::getLogger()->debug("Doing get object ids frame");

  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Object Ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }

  if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  unsigned int seqkey = frame->unpackInt();
  if(seqkey == 0xffffffff){
    //start new seqkey
    seqkey = ++currObjSeq;
  }
  
  unsigned int start = frame->unpackInt();
  unsigned int num = frame->unpackInt();
  
  if(seqkey != currObjSeq){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    curConnection->sendFrame(of);
    return;
  }
  
  unsigned int num_remain;

  if(num == 0xffffffff || start + num > visibleObjects.size()){
    num = visibleObjects.size() - start;
    num_remain = 0;
  }else{
    num_remain = visibleObjects.size() - start - num;
  }
  
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_ObjectIds_List);
  of->packInt(seqkey);
  of->packInt(num_remain);
  of->packInt(num);
  std::set<unsigned int>::iterator itcurr = visibleObjects.begin();
  advance(itcurr, start);
  for(unsigned int i = 0; i < num; i++){
    of->packInt(*itcurr);
    of->packInt64(Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime());
    Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
    ++itcurr;
  }
  curConnection->sendFrame(of);
}

void Player::processGetObjectIdsByPos(Frame* frame){
 Logger::getLogger()->debug("doing get object ids by pos frame");
  Frame *of = curConnection->createFrame(frame);
  if (frame->getDataLength() >= 36) {
    Vector3d pos;
    unsigned long long r;

    pos.unpack(frame);
    r = frame->unpackInt64();

    std::set<unsigned int> oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);

    for(std::set<unsigned int>::iterator vischk = oblist.begin(); vischk != oblist.end();){
      if(visibleObjects.find(*vischk) == visibleObjects.end()){
	std::set<unsigned int>::iterator temp = vischk;
	++temp;
	oblist.erase(vischk);
	vischk = temp;
      }else{
	++vischk;
      }
    }

    of->setType(ft03_ObjectIds_List);
    of->packInt(0);
    of->packInt(0);
    of->packInt(oblist.size());
    for(std::set<unsigned int>::iterator itcurr = oblist.begin(); itcurr != oblist.end(); ++itcurr){
      of->packInt(*itcurr);
      of->packInt64(Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime());
      Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
    }
  }else{
    of->createFailFrame(fec_FrameError, "Invalid frame");
  }
  curConnection->sendFrame(of);
}

void Player::processGetObjectIdsByContainer(Frame * frame){
  Logger::getLogger()->debug("doing get object ids by container frame");

  Frame *of = curConnection->createFrame(frame);
  if(frame->getDataLength() != 4){
    of->createFailFrame(fec_FrameError, "Invalid frame");
  }else{
    unsigned int objectID = frame->unpackInt();
    if(visibleObjects.find(objectID) != visibleObjects.end()){
      
        IGObject *o = Game::getGame()->getObjectManager()->getObject(objectID);
      
      if(o != NULL){
	std::set<unsigned int> contain = o->getContainedObjects();
	
	for(std::set<unsigned int>::iterator vischk = contain.begin(); vischk != contain.end();){
	  if(visibleObjects.find(*vischk) == visibleObjects.end()){
	    std::set<unsigned int>::iterator temp = vischk;
	    ++temp;
	    contain.erase(vischk);
	    vischk = temp;
	  }else{
	    ++vischk;
	  }
	}

	of->setType(ft03_ObjectIds_List);
	of->packInt(0);
	of->packInt(0);
	of->packInt(contain.size());
	for(std::set<unsigned int>::iterator itcurr = contain.begin(); itcurr != contain.end(); ++itcurr){
	  of->packInt(*itcurr);
            of->packInt64(Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime());
            Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
	}
        Game::getGame()->getObjectManager()->doneWithObject(objectID);
	
      }else{
	of->createFailFrame(fec_NonExistant, "No such Object");
      }

    }else{
      of->createFailFrame(fec_NonExistant, "No such Object");
    }
  }
  curConnection->sendFrame(of);
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
  
  if(visibleObjects.find(objectID) == visibleObjects.end()){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_NonExistant, "No such object");
    curConnection->sendFrame(of);
    return;
  }

  IGObject *o = Game::getGame()->getObjectManager()->getObject(objectID);
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
    Order *ord = Game::getGame()->getOrderManager()->getOrder(o, ordpos, pid);
    if (ord != NULL) {
      ord->createFrame(of, objectID, ordpos);
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not get Order");
    }
    Game::getGame()->getObjectManager()->doneWithObject(objectID);
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

		 if(visibleObjects.find(objectID) == visibleObjects.end()){
		   of->createFailFrame(fec_NonExistant, "No such object");
		   curConnection->sendFrame(of);
		   return;
		 }

                IGObject *o = Game::getGame()->getObjectManager()->getObject(objectID);
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
				
                            if (ord->inputFrame(frame, pid) && Game::getGame()->getOrderManager()->addOrder(ord, o, pos, pid)) {
				    of->setType(ft02_OK);
					of->packString("Order Added");
				} else {
					of->createFailFrame(fec_TempUnavailable, "Could not add order");
				}
			}
                Game::getGame()->getObjectManager()->doneWithObject(objectID);
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

  if(visibleObjects.find(objectID) == visibleObjects.end()){
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

  if(num_orders > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(num_orders);
    curConnection->sendFrame(seq);
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);
    int ordpos = frame->unpackInt();
    IGObject * obj = Game::getGame()->getObjectManager()->getObject(objectID);
    if (obj != NULL && Game::getGame()->getOrderManager()->removeOrder(obj, ordpos, pid)) {
      of->setType(ft02_OK);
      of->packString("Order removed");
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not remove Order");
    }
    Game::getGame()->getObjectManager()->doneWithObject(objectID);

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

void Player::processGetOrderTypes(Frame * frame){
  Logger::getLogger()->debug("doing get order types frame");
  
  Frame *of = curConnection->createFrame(frame);
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    of->createFailFrame(fec_FrameError, "Get order type frame isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }

  if(frame->getDataLength() != 12){
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  Game::getGame()->getOrderManager()->doGetOrderTypes(frame, of);
  
  curConnection->sendFrame(of);
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
  IGObject* theobject = Game::getGame()->getObjectManager()->getObject(obid);
  if(theobject == NULL){
    Logger::getLogger()->debug("The object the probed orders are for doesn't exist");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_NonExistant, "No such object");
    curConnection->sendFrame(of);
    return;
  }

  int pos = frame->unpackInt();
  
  Frame *of = curConnection->createFrame(frame);
  // See if we have a valid order
  Order *ord = Game::getGame()->getOrderManager()->createOrder(frame->unpackInt());
  if (ord == NULL) {
    of->createFailFrame(fec_NonExistant, "No such order type");
  }else if(theobject->getObjectData()->checkAllowedOrder(ord->getType(), pid)){
    
    ord->inputFrame(frame, pid);
    ord->createFrame(of, obid, pos);
    
  }else{
    
    Logger::getLogger()->debug("The order to be probed is not allowed on this object");
    of->createFailFrame(fec_PermUnavailable, "The order to be probed is not allowed on this object, try again");
    
  }
  Game::getGame()->getObjectManager()->doneWithObject(obid);
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
        Board* board = Game::getGame()->getBoardManager()->getBoard(boardid);
      board->packBoard(of);
    }else{
      //boards in the game object
      of->createFailFrame(fec_PermUnavailable, "No non-player boards yet");
    }
    curConnection->sendFrame(of);
  }
}

void Player::processGetBoardIds(Frame * frame){
   Logger::getLogger()->debug("Doing get board ids frame");
   
   if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Board Ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }

   if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  unsigned int seqkey = frame->unpackInt();
  if(seqkey == 0xffffffff){
    //start new seqkey
    seqkey = 0;
  }
    frame->unpackInt(); // starting number
    uint32_t numtoget = frame->unpackInt();

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_BoardIds_List);
  of->packInt(seqkey);
    if(numtoget == 0){
        of->packInt(1);
        of->packInt(0);
    }else{
  of->packInt(0);
  of->packInt(1);
  of->packInt(0); //personal board
  of->packInt64(Game::getGame()->getBoardManager()->getBoard(boardid)->getModTime());
    }
  
  curConnection->sendFrame(of);
}

void Player::processGetMessages(Frame * frame){
  Logger::getLogger()->debug("doing Get Messages frame");

  int lboardid = frame->unpackInt();
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
    //HACK
    // should depend on what the player should be allowed to see
    if(lboardid == 0)
        lboardid = boardid;
    currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard != NULL){
    for(int i = 0; i < nummsg; i++){
      Frame *of = curConnection->createFrame(frame);
      int msgnum = frame->unpackInt();

      currboard->packMessage(of, msgnum);

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

  int lboardid = frame->unpackInt();
  int pos = frame->unpackInt();

  Board * currboard;
    //HACK
    // should depend on what the player should be allowed to see
    if(lboardid == 0)
        lboardid = boardid;
    currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard != NULL){
    Message* msg = new Message();
        int msgtypes = frame->unpackInt(); // message type, no longer used
        for(int i = 0; i < msgtypes; i++){
            //have to clear the list of msgtypes
            // only filled under TP02
            frame->unpackInt();
        }
    msg->setSubject(std::string(frame->unpackString()));
    msg->setBody(std::string(frame->unpackString()));
        if(frame->getVersion() >= fv0_3){
            frame->unpackInt(); // turn created - overwritten by current turn number
            uint32_t numrefs = frame->unpackInt();
            for(uint32_t i = 0; i < numrefs; i++){
                int32_t rtype = frame->unpackInt();
                uint32_t rid = frame->unpackInt();
                msg->addReference(rtype, rid);
            }
        }
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

  int lboardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  if(nummsg > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(nummsg);
    curConnection->sendFrame(seq);
  }

  Board * currboard;
    //HACK
    // should depend on what the player should be allowed to see
    if(lboardid == 0)
        lboardid = boardid;
    currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard != NULL){
    for(int i = 0; i < nummsg; i++){
      Frame *of = curConnection->createFrame(frame);
      int msgnum = frame->unpackInt();

      if(currboard->removeMessage(msgnum)){
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

void Player::processGetResourceDescription(Frame * frame){
  Logger::getLogger()->debug("doing Get Resource Description frame");
  
  int numress = frame->unpackInt();
  if(numress > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numress);
    curConnection->sendFrame(seq);
  }
  
  if(numress == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no resource descriptions, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any resource descriptions, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numress; i++){
    Frame *of = curConnection->createFrame(frame);
    int rnum = frame->unpackInt();
    
    //There are no resources defined currently, so always fail.
    of->createFailFrame(fec_NonExistant, "No Resource Descriptions available");

    curConnection->sendFrame(of);
  }
}

void Player::processGetResourceTypes(Frame* frame){
  Logger::getLogger()->debug("doing Get Resource Types frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Resource Types isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }

   if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  unsigned int seqkey = frame->unpackInt();
  if(seqkey == 0xffffffff){
    //start new seqkey
    seqkey = 0;
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_ResType_List);
  of->packInt(seqkey);
  of->packInt(0);
  of->packInt(0); // no resource descriptions
 
  curConnection->sendFrame(of);
}

void Player::processGetPlayer(Frame* frame){
  Logger::getLogger()->debug("doing Get Player frame");
  
  int numplayers = frame->unpackInt();
  if(numplayers > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numplayers);
    curConnection->sendFrame(seq);
  }
  
  if(numplayers == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no players, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any players, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numplayers; i++){
    Frame *of = curConnection->createFrame(frame);
    int pnum = frame->unpackInt();
    if(pnum == 0){
      packFrame(of);
    }else{
        if(pnum != -1){
            Player* p = Game::getGame()->getPlayerManager()->getPlayer(pnum);
            if(p != NULL){
                p->packFrame(of);
            }else{
                of->createFailFrame(fec_NonExistant, "Player doesn't exist");
            }
        }else{
            of->createFailFrame(fec_NonExistant, "Player -1 doesn't exist, invalid player id");
        }
    }
    curConnection->sendFrame(of);
  }
}

void Player::processGetCategory(Frame* frame){
  Logger::getLogger()->debug("doing Get Category frame");

  int numcats = frame->unpackInt();
  if(numcats > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numcats);
    curConnection->sendFrame(seq);
  }
  
  if(numcats == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no categories, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any categories, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numcats; i++){
    Frame *of = curConnection->createFrame(frame);
    int catnum = frame->unpackInt();
    Category* cat = Game::getGame()->getDesignStore()->getCategory(catnum);
    if(cat == NULL){
      of->createFailFrame(fec_NonExistant, "No Such Category");
    }else{
      cat->packFrame(of);
    }
    curConnection->sendFrame(of);
  }

}

void Player::processGetCategoryIds(Frame* frame){
  Logger::getLogger()->debug("doing Get Category Ids frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Category ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }
  
  if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }
    frame->unpackInt(); //seqnum
    uint32_t snum = frame->unpackInt();
    uint32_t numtoget = frame->unpackInt();

  std::set<unsigned int> cids = Game::getGame()->getDesignStore()->getCategoryIds();
    if(snum > cids.size()){
        Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, cids.size());
        Frame *of = curConnection->createFrame(frame);
        of->createFailFrame(fec_NonExistant, "Starting number too high");
        curConnection->sendFrame(of);
        return;
    }
    if(numtoget > cids.size() - snum){
        numtoget = cids.size() - snum;
    }
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_CategoryIds_List);
  of->packInt(0);
    of->packInt(cids.size() - snum - numtoget);
    of->packInt(numtoget);
    std::set<unsigned int>::iterator itcurr = cids.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(0ll);
  }
 
  curConnection->sendFrame(of);
}

void Player::processGetDesign(Frame* frame){
  Logger::getLogger()->debug("doing Get Design frame");

  DesignStore* ds = Game::getGame()->getDesignStore();

  int numdesigns = frame->unpackInt();
  if(numdesigns > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numdesigns);
    curConnection->sendFrame(seq);
  }
  
  if(numdesigns == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no designs, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any designs, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numdesigns; i++){
    Frame *of = curConnection->createFrame(frame);
    int designnum = frame->unpackInt();
    Design* design = ds->getDesign(designnum);
    if(design == NULL || visibleDesigns.find(designnum) == visibleDesigns.end()){
       of->createFailFrame(fec_NonExistant, "No Such Design");
    }else{
      design->packFrame(of);
    }
    curConnection->sendFrame(of);
  }
}

void Player::processAddDesign(Frame* frame){
  Logger::getLogger()->debug("doing Add Design frame");

  Design* design = new Design();
  frame->unpackInt(); //designid, don't take, overwrite
  frame->unpackInt64(); //timestamp, discard
  frame->unpackInt(); //num of categories, had better be 1
  design->setCategoryId(frame->unpackInt());
  design->setName(std::string(frame->unpackString()));
  design->setDescription(std::string(frame->unpackString()));
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(pid);
  unsigned int tpid = frame->unpackInt();
  if(pid != tpid){
    Logger::getLogger()->debug("Odd, client sent wrong player id %d", tpid);
  }
  unsigned int numcomp = frame->unpackInt();
  std::map<unsigned int, unsigned int> comps;
  for(unsigned int i = 0; i < numcomp; i++){
        unsigned int compid = frame->unpackInt();
        comps[compid] = (frame->unpackInt());
  }
    design->setComponents(comps);
  //discard rest of frame

  DesignStore* ds = Game::getGame()->getDesignStore();

  Frame *of = curConnection->createFrame(frame);
  if(ds->addDesign(design)){
    of->setType(ft02_OK);
    of->packString("Design added");
  }else{
    of->createFailFrame(fec_FrameError, "Could not add design");
  }
  curConnection->sendFrame(of);
}

void Player::processModifyDesign(Frame* frame){
  Logger::getLogger()->debug("doing Modify Design frame");

  Design* design = new Design();
  design->setDesignId(frame->unpackInt());
  frame->unpackInt64(); //timestamp, discard
  frame->unpackInt(); //num of categories, had better be 1
  design->setCategoryId(frame->unpackInt());
  design->setName(std::string(frame->unpackString()));
  design->setDescription(std::string(frame->unpackString()));
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(pid);
  unsigned int tpid = frame->unpackInt();
  if(pid != tpid){
    Logger::getLogger()->debug("Odd, client sent wrong player id %d", tpid);
  }
  unsigned int numcomp = frame->unpackInt();
  std::map<unsigned int, unsigned int> comps;
  for(unsigned int i = 0; i < numcomp; i++){
        unsigned int compid = frame->unpackInt();
        comps[compid] = (frame->unpackInt());
  }
    design->setComponents(comps);
  //discard rest of frame

  DesignStore* ds = Game::getGame()->getDesignStore();

  Frame *of = curConnection->createFrame(frame);
  if(ds->modifyDesign(design)){
    of->setType(ft02_OK);
    of->packString("Design modified");
  }else{
    of->createFailFrame(fec_FrameError, "Could not modify design");
  }
  curConnection->sendFrame(of);
}

void Player::processGetDesignIds(Frame* frame){
 Logger::getLogger()->debug("doing Get Design Ids frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Design ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }
  
  if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }
  
    frame->unpackInt(); //seqnum
    uint32_t snum = frame->unpackInt();
    uint32_t numtoget = frame->unpackInt();
    if(snum > visibleDesigns.size()){
        Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, visibleDesigns.size());
        Frame *of = curConnection->createFrame(frame);
        of->createFailFrame(fec_NonExistant, "Starting number too high");
        curConnection->sendFrame(of);
        return;
    }
    if(numtoget > visibleDesigns.size() - snum){
        numtoget = visibleDesigns.size() - snum;
    }
    
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_DesignIds_List);
  of->packInt(0);
    of->packInt(visibleDesigns.size() - snum - numtoget);
    of->packInt(numtoget);
    std::set<unsigned int>::iterator itcurr = visibleDesigns.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(0ll);
  }
 
  curConnection->sendFrame(of);
}

void Player::processGetComponent(Frame* frame){
  Logger::getLogger()->debug("doing Get Component frame");

  DesignStore* ds = Game::getGame()->getDesignStore();

  int numcomps = frame->unpackInt();
  if(numcomps > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numcomps);
    curConnection->sendFrame(seq);
  }
  
  if(numcomps == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no components, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any components, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numcomps; i++){
    Frame *of = curConnection->createFrame(frame);
    int compnum = frame->unpackInt();
    Component* component = ds->getComponent(compnum);
    if(component == NULL || visibleComponents.find(compnum) == visibleComponents.end()){
      of->createFailFrame(fec_NonExistant, "No Such Component");
    }else{
      component->packFrame(of);
    }
    curConnection->sendFrame(of);
  }
}

void Player::processGetComponentIds(Frame* frame){
 Logger::getLogger()->debug("doing Get Component Ids frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Design ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }
  
    if(frame->getDataLength() != 12){
        Frame *of = curConnection->createFrame(frame);
        of->createFailFrame(fec_FrameError, "Invalid frame");
        curConnection->sendFrame(of);
        return;
    }
    frame->unpackInt(); //seqnum
    uint32_t snum = frame->unpackInt();
    uint32_t numtoget = frame->unpackInt();
    if(snum > visibleComponents.size()){
        Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, visibleComponents.size());
        Frame *of = curConnection->createFrame(frame);
        of->createFailFrame(fec_NonExistant, "Starting number too high");
        curConnection->sendFrame(of);
        return;
    }
    if(numtoget > visibleComponents.size() - snum){
        numtoget = visibleComponents.size() - snum;
    }
  
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_ComponentIds_List);
  of->packInt(0);
    of->packInt(visibleComponents.size() - snum - numtoget);
    of->packInt(numtoget);
    std::set<unsigned int>::iterator itcurr = visibleComponents.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(0ll);
  }
 
  curConnection->sendFrame(of);
}

void Player::processGetProperty(Frame* frame){
  Logger::getLogger()->debug("doing Get Property frame");

  DesignStore* ds = Game::getGame()->getDesignStore();

  int numprops = frame->unpackInt();
  if(numprops > 1){
    Frame *seq = curConnection->createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numprops);
    curConnection->sendFrame(seq);
  }
  
  if(numprops == 0){
    Frame *of = curConnection->createFrame(frame);
    Logger::getLogger()->debug("asked for no properties, silly client...");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any properties, try again");
    curConnection->sendFrame(of);
  }

  for(int i = 0; i < numprops; i++){
    Frame *of = curConnection->createFrame(frame);
    int propnum = frame->unpackInt();
    Property* property = ds->getProperty(propnum);
    if(property == NULL){
      of->createFailFrame(fec_NonExistant, "No Such Property");
    }else{
      property->packFrame(of);
    }
    curConnection->sendFrame(of);
  }
}

void Player::processGetPropertyIds(Frame* frame){
 Logger::getLogger()->debug("doing Get Property Ids frame");
  
  if(frame->getVersion() < fv0_3){
    Logger::getLogger()->debug("protocol version not high enough");
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Get Design ids isn't supported in this protocol");
    curConnection->sendFrame(of);
    return;
  }
  
  if(frame->getDataLength() != 12){
    Frame *of = curConnection->createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    curConnection->sendFrame(of);
    return;
  }

  std::set<unsigned int> propids = Game::getGame()->getDesignStore()->getPropertyIds();
    frame->unpackInt(); //seqnum
    uint32_t snum = frame->unpackInt();
    uint32_t numtoget = frame->unpackInt();
    if(snum > propids.size()){
        Logger::getLogger()->debug("Starting number too high, snum = %d, size = %d", snum, propids.size());
        Frame *of = curConnection->createFrame(frame);
        of->createFailFrame(fec_NonExistant, "Starting number too high");
        curConnection->sendFrame(of);
        return;
    }
    if(numtoget > propids.size() - snum){
        numtoget = propids.size() - snum;
    }
    
  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_PropertyIds_List);
  of->packInt(0);
    of->packInt(propids.size() - snum - numtoget);
    of->packInt(numtoget);
    std::set<unsigned int>::iterator itcurr = propids.begin();
    std::advance(itcurr, snum);
    for(uint32_t i = 0; i < numtoget; i++, ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(0ll);
  }
 
  curConnection->sendFrame(of);
}
