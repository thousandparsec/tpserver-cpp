/*  PlayerAgent object, processing and frame handling
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <ctime>

// #include "string.h"

#include "playerconnection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"
#include "objectview.h"
#include "orderqueue.h"
#include "order.h"
#include "vector3d.h"
#include "board.h"
#include "message.h"
#include "objectmanager.h"
#include "objecttypemanager.h"
#include "ordermanager.h"
#include "boardmanager.h"
#include "playermanager.h"
#include "designstore.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"
#include "resourcedescription.h"
#include "resourcemanager.h"
#include "player.h"
#include "playerview.h"
#include "turntimer.h"

#include "orderqueueobjectparam.h"

#include "playeragent.h"

PlayerAgent::PlayerAgent( PlayerConnection* connection, Player* nplayer )
  : curConnection ( connection ), player( nplayer ){
}

PlayerAgent::~PlayerAgent(){
}

PlayerConnection *PlayerAgent::getConnection() const{
  return curConnection;
}

Player *PlayerAgent::getPlayer() const{
  return player;
}

void PlayerAgent::processIGFrame(Frame * frame){
  if(player == NULL){
    Logger::getLogger()->warning("No Player for PlayerAgent to work on behalf of");
    return;
  }
  DEBUG("IG Frame processor");

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

    case ft04_TurnFinished:
      processTurnFinished(frame);
      break;

    case ft04_ObjectDesc_Get:
      processGetObjectDesc(frame);
      break;

    case ft04_ObjectTypes_Get:
      processGetObjectTypes(frame);
      break;
    case ft04_PlayerIds_Get:
      processGetPlayerIds(frame);
      break;
    default:
      WARNING("PlayerAgent: Discarded frame, not processed, was type %d", frame->getType());
      // Send a failed frame
      curConnection->sendFail(frame,fec_ProtocolError, "Did not understand that frame type.");
      break;
  }
}



void PlayerAgent::processPermDisabled(Frame * frame){
  DEBUG("doing a frame that is disabled");
  curConnection->sendFail(frame,fec_PermUnavailable, "Server does not support this frame type");
}

void PlayerAgent::processGetObjectById (Frame * frame){
  DEBUG ( "doing get object by id frame" );

  int len = queryCheck( frame );
  if ( len == 0 ) return;

  // Object frames
  for ( int i=0 ; i < len; ++i ){
    uint32_t objectID = frame->unpackInt();
    Frame* of = curConnection->createFrame ( frame );
    player->getPlayerView()->processGetObject(objectID, of);
    curConnection->sendFrame ( of );
  }
}

void PlayerAgent::processGetObjectByPos(Frame * frame)
{
  DEBUG("doing get object by pos frame");

  if (!lengthCheckMin( frame, 36 ) ) return;

  Frame *of;
  Vector3d pos;
  uint64_t r;
  pos.unpack(frame);
  r = frame->unpackInt64();

  IdSet oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);

  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();

  for(IdSet::iterator vischk = oblist.begin(); vischk != oblist.end();){
    if(visibleObjects.find(*vischk) == visibleObjects.end()){
      IdSet::iterator temp = vischk;
      ++temp;
      oblist.erase(vischk);
      vischk = temp;
    }else{
      ++vischk;
    }
  }

  curConnection->sendSequence( frame, oblist.size() );

  IdSet::iterator obCurr = oblist.begin();
  for( ; obCurr != oblist.end(); ++obCurr) {
    of = curConnection->createFrame(frame);
    player->getPlayerView()->processGetObject(*obCurr, of);
    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processGetObjectIds(Frame * frame){
  DEBUG("Doing get object ids frame");
  Frame *of = curConnection->createFrame(frame);
  player->getPlayerView()->processGetObjectIds(frame, of);
  curConnection->sendFrame(of);
}


void PlayerAgent::processGetObjectIdsByPos(Frame* frame){
  DEBUG("doing get object ids by pos frame");
  Frame *of = curConnection->createFrame(frame);
  if (!lengthCheckMin( frame, 36 ) ) return;
  Vector3d pos;
  uint64_t r;

  pos.unpack(frame);
  r = frame->unpackInt64();

  IdSet oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);
  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();
  for(IdSet::iterator vischk = oblist.begin(); vischk != oblist.end();){
    if(visibleObjects.find(*vischk) == visibleObjects.end()){
      IdSet::iterator temp = vischk;
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
  for(IdSet::iterator itcurr = oblist.begin(); itcurr != oblist.end(); ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime());
    Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetObjectIdsByContainer(Frame * frame){
  DEBUG("doing get object ids by container frame");
  if (!lengthCheck( frame, 4 ) ) return;
  Frame *of = curConnection->createFrame(frame);
  uint32_t objectID = frame->unpackInt();
  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();
  if(visibleObjects.find(objectID) != visibleObjects.end()){

    IGObject *o = Game::getGame()->getObjectManager()->getObject(objectID);

    if(o != NULL){
      IdSet contain = o->getContainedObjects();

      for(IdSet::iterator vischk = contain.begin(); vischk != contain.end();){
        if(visibleObjects.find(*vischk) == visibleObjects.end()){
          IdSet::iterator temp = vischk;
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
      for(IdSet::iterator itcurr = contain.begin(); itcurr != contain.end(); ++itcurr){
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
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetObjectDesc(Frame * frame){
  DEBUG("Doing get OrderDesc");
  if ( !versionCheck( frame, fv0_4 ) ) return;
  int len = queryCheck( frame );
  if ( len == 0 ) return;

  // Object frames
  for ( int i=0 ; i < len; ++i ){
    uint32_t objecttype = frame->unpackInt();
    Frame* of = curConnection->createFrame(frame);
    Game::getGame()->getObjectTypeManager()->doGetObjectDesc(objecttype, of);
    curConnection->sendFrame ( of );
  }
}

void PlayerAgent::processGetObjectTypes(Frame * frame){
  DEBUG("Doing get OrderTypes list");
  if ( !versionCheck( frame, fv0_4 ) ) return;
  if ( !lengthCheck( frame, 20 ) ) return;
  Frame *of = curConnection->createFrame(frame);
  Game::getGame()->getObjectTypeManager()->doGetObjectTypes(frame, of);
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetOrder(Frame * frame){
  DEBUG("Doing get order frame");
  if ( !lengthCheckMin( frame, 12 ) ) return;
  uint32_t orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject* ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object");
      return;
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object OrderQueue");
      return;
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    curConnection->sendFail(frame,fec_NonExistant, "No such Order Queue");
    return;
  }

  int num_orders = frame->unpackInt();

  if ( !lengthCheck( frame, 8 + 4 * num_orders ) ) return;

  if(num_orders > 1) {
    DEBUG("Got multiple orders, returning a sequence");
    curConnection->sendSequence(frame,num_orders);
  } else {
    DEBUG("Got single orders, returning one object");
  }

  if(num_orders == 0){
    curConnection->sendFail(frame,fec_FrameError,"No orders to get");
    return;
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);

    int ordpos = frame->unpackInt();
    Order *ord = orderqueue->getOrder(ordpos, player->getID());
    if (ord != NULL) {
      ord->createFrame(of, ordpos);
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not get Order");
    }
    curConnection->sendFrame(of);
  }


}


void PlayerAgent::processAddOrder(Frame * frame){
  DEBUG("doing add order frame");
  Frame *of = curConnection->createFrame(frame);
  if (frame->getDataLength() >= 8) {

    // See if we have a valid orderqueue id

    uint32_t orderqueueid = frame->unpackInt();

    if(frame->getVersion() <= fv0_3){
      IGObject* ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
      if(ob == NULL){
        curConnection->sendFail(frame,fec_NonExistant, "No such Object");
        return;
      }
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
      if(oqop == NULL){
        curConnection->sendFail(frame,fec_NonExistant, "No such Object OrderQueue");
        return;
      }
      orderqueueid = oqop->getQueueId();
    }

    OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
    if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
      curConnection->sendFail(frame,fec_NonExistant, "No such Order Queue");
      return;
    }

    // Order Slot
    int pos = frame->unpackInt();

    // See if we have a valid order
    Order *ord = Game::getGame()->getOrderManager()->createOrder(frame->unpackInt());
    if (ord == NULL) {
      of->createFailFrame(fec_NonExistant, "No such order type");
    } else {
      ord->setOrderQueueId(orderqueueid);
      Result r = ord->inputFrame(frame, player->getID());
      if (r){
        if(orderqueue->addOrder(ord, pos, player->getID())) {
          if(of->getVersion() >= fv0_4){
            ord->createFrame(of, pos);
          }else{
            of->setType(ft02_OK);
            of->packString("Order Added");
          }
          //update ObjectView
          uint32_t objid = orderqueue->getObjectId();
          if(objid != 0){
            ObjectView* obv = player->getPlayerView()->getObjectView(objid);
            obv->touchModTime();
            player->getPlayerView()->updateObjectView(objid);
          }
        } else {
          of->createFailFrame(fec_TempUnavailable, "Not allowed to add that order type.");
        }
      }else{
        // FIXME: This isn't always a FrameError really...
        of->createFailFrame(fec_FrameError, (std::string("Could not add order, ") + r).c_str());
      }
    }
  } else {
    of->createFailFrame(fec_FrameError, "Invalid frame, Add Order, too short");
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processRemoveOrder(Frame * frame){
  DEBUG("doing remove order frame");

  if ( !lengthCheckMin( frame, 12 ) ) return;

  int orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject* ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object");
      return;
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object OrderQueue");
      return;
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    curConnection->sendFail(frame,fec_NonExistant, "No such OrderQueue");
    return;
  }

  int num_orders = frame->unpackInt();

  if ( !lengthCheckMin( frame, 8 + 4 * num_orders  ) ) return;

  if(num_orders > 1){
    curConnection->sendSequence(frame,num_orders);
  }

  for(int i = 0; i < num_orders; i++){
    Frame *of = curConnection->createFrame(frame);
    int ordpos = frame->unpackInt();
    if (orderqueue->removeOrder(ordpos, player->getID())) {
      of->setType(ft02_OK);
      of->packString("Order removed");
      //update ObjectView
      uint32_t objid = orderqueue->getObjectId();
      if(objid != 0){
        ObjectView* obv = player->getPlayerView()->getObjectView(objid);
        obv->touchModTime();
        player->getPlayerView()->updateObjectView(objid);
      }
    } else {
      of->createFailFrame(fec_TempUnavailable, "Could not remove Order");
    }

    curConnection->sendFrame(of);

  }

}


void PlayerAgent::processDescribeOrder(Frame * frame)
{
  DEBUG("doing describe order frame");

  int numdesc = queryCheck( frame );
  if ( numdesc == 0 ) return;

  for(int i = 0; i < numdesc; i++){
    Frame *of = curConnection->createFrame(frame);
    int ordertype = frame->unpackInt();
    Game::getGame()->getOrderManager()->describeOrder(ordertype, of);
    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processGetOrderTypes(Frame * frame){
  DEBUG("doing get order types frame");

  Frame *of = curConnection->createFrame(frame);

  if ( !versionCheck(frame,fv0_3) ) return;

  if(frame->getDataLength() != 12 && frame->getVersion() == fv0_3) {
    of->createFailFrame(fec_FrameError, "Invalid frame, Get Order Types (TP03), Frame too short (<12 bytes)");
  } else if (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4) {
    of->createFailFrame(fec_FrameError, "Invalid frame, Get Order Types (TP04), Frame too short (<20 bytes)");
  } else {
    Game::getGame()->getOrderManager()->doGetOrderTypes(frame, of);
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processProbeOrder(Frame * frame){
  DEBUG("doing probe order frame");

  if ( !versionCheck(frame,fv0_3) ) return;
  if ( !lengthCheckMin( frame, 8) ) return;

  int orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject* ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object");
      return;
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      curConnection->sendFail(frame,fec_NonExistant, "No such Object OrderQueue");
      return;
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    curConnection->sendFail(frame,fec_NonExistant, "No such Order Queue");
    return;
  }

  int pos = frame->unpackInt();

  Frame *of = curConnection->createFrame(frame);
  // See if we have a valid order
  Order *ord = Game::getGame()->getOrderManager()->createOrder(frame->unpackInt());
  if (ord == NULL) {
    of->createFailFrame(fec_NonExistant, "No such order type");
  }else if(orderqueue->checkOrderType(ord->getType(), player->getID())){
    ord->setOrderQueueId(orderqueueid);
    if(ord->inputFrame(frame, player->getID())){
      ord->createFrame(of, pos);
    }else{
      of->createFailFrame(fec_FrameError, "Order could not be unpacked correctly, invalid order");
      DEBUG("Probe Order, could not unpack order");
    }

  }else{

    DEBUG("The order to be probed is not allowed on this object");
    of->createFailFrame(fec_PermUnavailable, "The order to be probed is not allowed on this object, try again");

  }
  delete ord;
  curConnection->sendFrame(of);

}

void PlayerAgent::processGetBoards(Frame * frame){
  DEBUG("doing Get Boards frame");

  int numboards = queryCheck( frame );
  if ( numboards == 0 ) return;

  for(int i = 0; i < numboards; i++){
    Frame *of = curConnection->createFrame(frame);
    uint32_t boardnum = frame->unpackInt();
    if(boardnum == 0 || boardnum == player->getBoardId()){
      Board::Ptr board = Game::getGame()->getBoardManager()->getBoard(player->getBoardId());
      board->pack(of);
    }else{
      //boards in the game object
      of->createFailFrame(fec_PermUnavailable, "No non-player boards yet");
    }
    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processGetBoardIds(Frame* frame){
  DEBUG("Doing get board ids frame");

  if ( !versionCheck(frame,fv0_3) ) return;

  if((frame->getDataLength() != 12 && frame->getVersion() == fv0_3) || (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4)){
    curConnection->sendFail(frame,fec_FrameError, "Invalid frame, Get Board Ids, Frame too short");
    return;
  }

  uint32_t seqkey = frame->unpackInt();
  if(seqkey == UINT32_NEG_ONE){
    //start new seqkey
    seqkey = 0;
  }
  /*uint32_t snum =*/ frame->unpackInt(); // starting number
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_BoardIds_List);
  of->packInt(seqkey);

  if(numtoget == 0){
    of->packInt(1); // On ID left to get
    of->packInt(0); // Zero sized list
  } else {
    of->packInt(0);
    of->packInt(1);
    of->packInt(0); //personal board
    of->packInt64(Game::getGame()->getBoardManager()->getBoard(player->getBoardId())->getModTime());
  }

  if(frame->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetMessages(Frame * frame){
  DEBUG("doing Get Messages frame");

  if ( !lengthCheckMin( frame, 8 ) ) return;

  int lboardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  if ( !lengthCheckMin( frame, 8 + 4 * nummsg ) ) return;

  if(nummsg > 1){
    curConnection->sendSequence(frame,nummsg);
  }

  if(nummsg == 0){
    curConnection->sendFail(frame, fec_FrameError, "No messages to get");
    return;
  }

  Board::Ptr currboard;
  //HACK
  // should depend on what the player should be allowed to see
  if(lboardid == 0)
    lboardid = player->getBoardId();
  currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard.get() != NULL){
    for(int i = 0; i < nummsg; i++){
      Frame *of = curConnection->createFrame(frame);
      int msgnum = frame->unpackInt();

      currboard->packMessage(of, msgnum);

      curConnection->sendFrame(of);

    }

  }else{
    curConnection->sendFail(frame,fec_NonExistant, "Board does not exist");
  }

}

void PlayerAgent::processPostMessage(Frame * frame){
  DEBUG("doing Post Messages frame");

  Frame *of = curConnection->createFrame(frame);

  if ( !lengthCheckMin( frame, 28 ) ) return;

  int lboardid = frame->unpackInt();
  int pos = frame->unpackInt();

  Board::Ptr currboard;
  //HACK
  // should depend on what the player should be allowed to see
  if(lboardid == 0)
    lboardid = player->getBoardId();
  currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard != NULL){
    Message::Ptr msg( new Message() );
    int msgtypes = frame->unpackInt(); // message type, no longer used
    for(int i = 0; i < msgtypes; i++){
      //have to clear the list of msgtypes
      // only filled under TP02
      frame->unpackInt();
    }
    msg->setSubject(frame->unpackStdString());
    msg->setBody(frame->unpackStdString());
    if(frame->getVersion() >= fv0_3){
      frame->unpackInt(); // turn created - overwritten by current turn number
      uint32_t numrefs = frame->unpackInt();
      for(uint32_t i = 0; i < numrefs; i++){
        int32_t rtype = frame->unpackInt();
        uint32_t rid = frame->unpackInt();
        msg->addReference((RefSysType)rtype, rid);
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

void PlayerAgent::processRemoveMessages(Frame * frame){
  DEBUG("doing Remove Messages frame");

  if ( !lengthCheckMin( frame, 8 ) ) return;

  int lboardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  if ( !lengthCheckMin( frame, 8 + 4 * nummsg ) ) return;

  if(nummsg > 1){
    curConnection->sendSequence(frame, nummsg);
  }

  Board::Ptr currboard;
  //HACK
  // should depend on what the player should be allowed to see
  if(lboardid == 0)
    lboardid = player->getBoardId();
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
    curConnection->sendFail(frame,fec_NonExistant, "Board does not exist");
  }

}

void PlayerAgent::processGetResourceDescription(Frame * frame){
  DEBUG("doing Get Resource Description frame");

  int numress = queryCheck( frame );
  if ( numress == 0 ) return;

  for(int i = 0; i < numress; i++){
    Frame *of = curConnection->createFrame(frame);
    int rnum = frame->unpackInt();

    const ResourceDescription * res = Game::getGame()->getResourceManager()->getResourceDescription(rnum);
    if(res != NULL){
      res->packFrame(of);
    }else{
      of->createFailFrame(fec_NonExistant, "No Resource Descriptions available");
    }

    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processGetResourceTypes(Frame* frame){
  DEBUG("doing Get Resource Types frame");

  if ( !versionCheck(frame,fv0_3) ) return;

  if((frame->getDataLength() != 12 && frame->getVersion() == fv0_3) || (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4)){
    curConnection->sendFail(frame,fec_FrameError, "Invalid frame, Get Resource Types, Frame too short");
    return;
  }

  uint32_t seqkey = frame->unpackInt();
  if(seqkey == UINT32_NEG_ONE){
    //start new seqkey
    seqkey = 0;
  }

  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  ResourceManager* rm = Game::getGame()->getResourceManager();
  IdSet idset = rm->getAllIds();

  IdModList modlist;
  for(IdSet::iterator itcurr = idset.begin();
      itcurr != idset.end(); ++itcurr){
    const ResourceDescription * res = rm->getResourceDescription(*itcurr);
    if(fromtime == UINT64_NEG_ONE || res->getModTime() > fromtime){
      modlist[*itcurr] = res->getModTime();
    }
  }

  if(snum > modlist.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", snum, modlist.size());
    curConnection->sendFail(frame,fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlist.size() - snum){
    numtoget = modlist.size() - snum;
  }

  if(numtoget > MAX_ID_LIST_SIZE + ((frame->getVersion() < fv0_4)? 1 : 0)){
    DEBUG("Number of items to get too high, numtoget = %d", numtoget);
    curConnection->sendFail(frame,fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_ResType_List);
  of->packInt(seqkey);
  of->packIdModList(modlist,numtoget,snum);
  if(of->getVersion() >= fv0_4) of->packInt64(fromtime);
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetPlayer(Frame* frame){
  DEBUG("doing Get Player frame");

  int numplayers = queryCheck( frame );
  if ( numplayers == 0 ) return;

  for(int i = 0; i < numplayers; i++){
    Frame *of = curConnection->createFrame(frame);
    int pnum = frame->unpackInt();
    if(pnum == 0){
      player->packFrame(of);
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

void PlayerAgent::processGetPlayerIds(Frame* frame){
  DEBUG("doing Get Resource Types frame");

  if ( !versionCheck(frame,fv0_4) ) return;

  if ( !lengthCheck( frame, 20 ) ) return;

  uint32_t seqkey = frame->unpackInt();
  if(seqkey == UINT32_NEG_ONE){
    //start new seqkey
    seqkey = 0;
  }

  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;

  fromtime = frame->unpackInt64();

  PlayerManager* pm = Game::getGame()->getPlayerManager();
  std::set<playerid_t> idset = pm->getAllIds();

  IdModList modlist;
  for(std::set<playerid_t>::iterator itcurr = idset.begin();
      itcurr != idset.end(); ++itcurr){
    const Player * pl = pm->getPlayer(*itcurr);
    if(fromtime == UINT64_NEG_ONE || pl->getModTime() > fromtime){
      modlist[*itcurr] = pl->getModTime();
    }
  }

  if(snum > modlist.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", snum, modlist.size());
    curConnection->sendFail(frame,fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlist.size() - snum){
    numtoget = modlist.size() - snum;
  }

  if(numtoget > MAX_ID_LIST_SIZE){
    DEBUG("Number of items to get too high, numtoget = %d", numtoget);
    curConnection->sendFail(frame,fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft04_PlayerIds_List);
  of->packInt(seqkey);
  of->packIdModList(modlist,numtoget,snum);
  of->packInt64(fromtime);
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetCategory(Frame* frame){
  DEBUG("doing Get Category frame");

  int numcats = queryCheck( frame );
  if ( numcats == 0 ) return;

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

void PlayerAgent::processGetCategoryIds(Frame* frame){
  DEBUG("doing Get Category Ids frame");

  if ( !versionCheck(frame,fv0_3) ) return;

  if((frame->getDataLength() != 12 && frame->getVersion() == fv0_3) || (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4)){
    curConnection->sendFail(frame,fec_FrameError, "Invalid frame, Get Categories Ids, Frame too short");
    return;
  }

  frame->unpackInt(); //seqnum
  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }


  DesignStore *ds = Game::getGame()->getDesignStore();
  IdSet cids = ds->getCategoryIds();

  IdModList modlist;
  for(IdSet::iterator itcurr = cids.begin();
      itcurr != cids.end(); ++itcurr){
    Category * cat = ds->getCategory(*itcurr);
    if(fromtime == UINT64_NEG_ONE || cat->getModTime() > fromtime){
      modlist[*itcurr] = cat->getModTime();
    }
  }

  if(snum > modlist.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", snum, modlist.size());
    curConnection->sendFail(frame,fec_NonExistant, "Starting number too high");
    return;
  }

  if(numtoget > modlist.size() - snum){
    numtoget = modlist.size() - snum;
  }

  if(numtoget > MAX_ID_LIST_SIZE + ((frame->getVersion() < fv0_4)? 1 : 0)){
    DEBUG("Number of items to get too high, numtoget = %d", numtoget);
    curConnection->sendFail(frame,fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_CategoryIds_List);
  of->packInt(0); // seqkey
  of->packIdModList(modlist,numtoget,snum);
  
  if(of->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }

  curConnection->sendFrame(of);
}

void PlayerAgent::processGetDesign(Frame* frame){
  DEBUG("doing Get Design frame");

  int numdesigns = queryCheck( frame );
  if ( numdesigns == 0 ) return;

  for(int i = 0; i < numdesigns; i++){
    Frame *of = curConnection->createFrame(frame);
    int designnum = frame->unpackInt();
    player->getPlayerView()->processGetDesign(designnum, of);
    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processAddDesign(Frame* frame){
  DEBUG("doing Add Design frame");

  if ( !lengthCheckMin( frame, 40 ) ) return;

  Design* design = new Design();
  frame->unpackInt(); //designid, don't take, overwrite
  frame->unpackInt64(); //timestamp, discard
  int numcats = frame->unpackInt(); //num of categories, had better be 1
  if(numcats >= 1){
    design->setCategoryId(frame->unpackInt());
    numcats--;
  }else{
    design->setCategoryId(1); //TODO should check against components
  }
  for(int i = 0; i < numcats; i++){
    frame->unpackInt();
  }
  design->setName(frame->unpackStdString());
  design->setDescription(frame->unpackStdString());
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(player->getID());
  uint32_t tpid = frame->unpackInt();
  if(player->getID() != tpid){
    DEBUG("Odd, client sent wrong player id %d", tpid);
  }
  uint32_t numcomp = frame->unpackInt();
  std::map<uint32_t, uint32_t> comps;
  for(uint32_t i = 0; i < numcomp; i++){
    uint32_t compid = frame->unpackInt();
    comps[compid] = (frame->unpackInt());
  }
  design->setComponents(comps);
  //discard rest of frame

  DesignStore* ds = Game::getGame()->getDesignStore();

  Frame *of = curConnection->createFrame(frame);
  if(ds->addDesign(design)){
        player->getPlayerView()->processGetDesign(design->getDesignId(), of);
  }else{
    of->createFailFrame(fec_FrameError, "Could not add design");
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processModifyDesign(Frame* frame){
  DEBUG("doing Modify Design frame");

  if ( !lengthCheckMin( frame, 40 ) ) return;

  Design* design = new Design();
  design->setDesignId(frame->unpackInt());
  frame->unpackInt64(); //timestamp, discard
  int numcats = frame->unpackInt(); //num of categories, had better be 1
  if(numcats >= 1){
    design->setCategoryId(frame->unpackInt());
    numcats--;
  }else{
    design->setCategoryId(1); //TODO should check against components
  }
  for(int i = 0; i < numcats; i++){
    frame->unpackInt();
  }
  design->setName(frame->unpackStdString());
  design->setDescription(frame->unpackStdString());
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(player->getID());
  uint32_t tpid = frame->unpackInt();
  if(player->getID() != tpid){
    DEBUG("Odd, client sent wrong player id %d", tpid);
  }
  uint32_t numcomp = frame->unpackInt();
  std::map<uint32_t, uint32_t> comps;
  for(uint32_t i = 0; i < numcomp; i++){
    uint32_t compid = frame->unpackInt();
    comps[compid] = (frame->unpackInt());
  }
  design->setComponents(comps);
  //discard rest of frame

  DesignStore* ds = Game::getGame()->getDesignStore();

  Frame *of = curConnection->createFrame(frame);
  if(ds->modifyDesign(design)){
        player->getPlayerView()->processGetDesign(design->getDesignId(), of);
  }else{
    of->createFailFrame(fec_FrameError, "Could not modify design");
  }
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetDesignIds(Frame* frame){
  Frame *of = curConnection->createFrame(frame);
  player->getPlayerView()->processGetDesignIds(frame, of);
  curConnection->sendFrame(of);
}

void PlayerAgent::processGetComponent(Frame* frame){
  DEBUG("doing Get Component frame");

  int numcomps = queryCheck( frame );
  if ( numcomps == 0 ) return;

  for(int i = 0; i < numcomps; i++){
    Frame *of = curConnection->createFrame(frame);
    int compnum = frame->unpackInt();
    player->getPlayerView()->processGetComponent(compnum, of);
    curConnection->sendFrame(of);
  }
}

void PlayerAgent::processGetComponentIds(Frame* frame){
  Frame *of = curConnection->createFrame(frame);
  player->getPlayerView()->processGetComponentIds(frame, of);

  curConnection->sendFrame(of);
}

void PlayerAgent::processGetProperty(Frame* frame){
  DEBUG("doing Get Property frame");

  DesignStore* ds = Game::getGame()->getDesignStore();

  int numprops = queryCheck( frame );
  if ( numprops == 0 ) return;
  curConnection->sendSequence(frame,numprops);

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

void PlayerAgent::processGetPropertyIds(Frame* frame){
  DEBUG("doing Get Property Ids frame");

  if(frame->getVersion() < fv0_3){
    DEBUG("protocol version not high enough");
    curConnection->sendFail(frame,fec_FrameError, "Get Design ids isn't supported in this protocol");
    return;
  }

  if((frame->getDataLength() != 12 && frame->getVersion() == fv0_3) || (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4)){
    curConnection->sendFail(frame,fec_FrameError, "Invalid frame, Get Property Ids, Frame too short");
    return;
  }

  IdSet propids = Game::getGame()->getDesignStore()->getPropertyIds();
  frame->unpackInt(); //seqnum
  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  DesignStore *ds = Game::getGame()->getDesignStore();

  IdModList modlist;
  for(IdSet::iterator itcurr = propids.begin();
      itcurr != propids.end(); ++itcurr){
    Property * prop = ds->getProperty(*itcurr);
    if(fromtime == UINT64_NEG_ONE || prop->getModTime() > fromtime){
      modlist[*itcurr] = prop->getModTime();
    }
  }

  if(snum > modlist.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", snum, modlist.size());
    curConnection->sendFail(frame,fec_NonExistant, "Starting number too high");
    return;
  }
  if(numtoget > modlist.size() - snum){
    numtoget = modlist.size() - snum;
  }

  if(numtoget > MAX_ID_LIST_SIZE + ((frame->getVersion() < fv0_4)? 1 : 0)){
    DEBUG("Number of items to get too high, numtoget = %d", numtoget);
    curConnection->sendFail(frame,fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft03_PropertyIds_List);
  of->packInt(0);
  of->packIdModList(modlist,numtoget,snum);

  if(of->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }

  curConnection->sendFrame(of);
}

void PlayerAgent::processTurnFinished(Frame* frame){
  DEBUG("doing Done Turn frame");

  if(frame->getVersion() < fv0_4){
    DEBUG("Turn Finished Frame: protocol version not high enough, but continuing anyway");
    //     Frame *of = curConnection->createFrame(frame);
    //     of->createFailFrame(fec_FrameError, "Finished Turn frame isn't supported in this protocol");
    //     curConnection->sendFrame(of);
    //     return;
  }

  Game::getGame()->getTurnTimer()->playerFinishedTurn(player->getID());

  Frame *of = curConnection->createFrame(frame);
  of->setType(ft02_OK);
  of->packString("Thanks for letting me know you have finished your turn.");
  curConnection->sendFrame(of);

}

bool PlayerAgent::versionCheck( Frame* frame, ProtocolVersion min_version )
{
  if(frame->getVersion() < min_version ){
    DEBUG("Frame protocol version not high enough");
    curConnection->sendFail(frame,fec_FrameError, "This feature isn't supported on this protocol version");
    return false;
  }
  return true;
}

bool PlayerAgent::lengthCheck( Frame* frame, uint32_t length )
{
  if ( frame->getDataLength() != (int)length ) {
    DEBUG("Frame of invalid size");
    curConnection->sendFail(frame,fec_FrameError, "Frame is of invalid size");
    return false;
  }
  return true;
}

bool PlayerAgent::lengthCheckMin( Frame* frame, uint32_t length )
{
  if ( frame->getDataLength() < (int)length ) {
    DEBUG("Frame is too short");
    curConnection->sendFail(frame,fec_FrameError, "Frame is too short");
    return false;
  }
  return true;
}

int PlayerAgent::queryCheck( Frame* frame )
{
  if ( !lengthCheckMin( frame, 4 ) ) return 0;
  int result = frame->unpackInt();
  if ( !lengthCheckMin( frame, 4 + 4 * result ) ) return 0;

  if ( result <= 0 ) {
    DEBUG( "Asked for no data, silly client... " );
    curConnection->sendFail(frame,fec_NonExistant, "You didn't ask for any data, try again");
    return 0;
  }
  curConnection->sendSequence( frame, result );

  return result;
}
