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
#include "logging.h"
#include "game.h"
#include "object.h"
#include "objectview.h"
#include "objecttype.h"
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
#include <sys/socket.h>

PlayerAgent::PlayerAgent( PlayerConnection::Ptr connection, Player::Ptr nplayer )
  : ref_connection ( connection ), temp_connection(), player(nplayer) {
}

PlayerAgent::~PlayerAgent(){
}

Player::Ptr PlayerAgent::getPlayer() const{
  return player;
}

void PlayerAgent::processIGFrame( InputFrame::Ptr frame ){
  if(player == NULL){
    WARNING("No Player for PlayerAgent to work on behalf of");
    return;
  }
  DEBUG("IG Frame processor");

  // Lock our temporary connection
  temp_connection = ref_connection.lock();
  if ( !temp_connection ) {
    ERROR("No PlayerConnection for PlayerAgent to work on behalf of!");
    return;
  }

  try {
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
    case ft04_OrderQueue_Get:
      processGetOrderQueue(frame);
      break;
    case ft04_OrderQueueIds_Get:
      processGetOrderQueueIds(frame);
      break;
    default:
      WARNING("PlayerAgent: Discarded frame, not processed, was type %d", frame->getType());
      throw FrameException( fec_ProtocolError, "Did not understand that frame type.");
      break;
  }
  } catch(...) {
    // release connection ownership
    temp_connection.reset();
    throw;
  }
  // release connection ownership
  temp_connection.reset();
}



void PlayerAgent::processPermDisabled( InputFrame::Ptr frame ){
  DEBUG("doing a frame that is disabled");
  throw FrameException( fec_PermUnavailable, "Server does not support this frame type");
}

void PlayerAgent::processGetObjectById ( InputFrame::Ptr frame ){
  DEBUG ( "doing get object by id frame" );

  int len = queryCheck( frame );

  // Object frames
  for ( int i=0 ; i < len; ++i ){
    uint32_t objectID = frame->unpackInt();
    ObjectView::Ptr object = player->getPlayerView()->getObjectView(objectID);
    OutputFrame::Ptr of;
    if ( !object ){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Object, objectID));
      temp_connection->sendFail(frame, fec_NonExistant, "No Such Object", reflist);
    }else{
        of = temp_connection->createFrame ( frame );
        try{
            object->packFrame( of, player->getId() );
            temp_connection->sendFrame ( of );
        }catch(FrameException e){
            temp_connection->sendFail(frame, e);
        }
    }
  }
}

void PlayerAgent::processGetObjectByPos( InputFrame::Ptr frame )
{
  DEBUG("doing get object by pos frame");

  lengthCheckMin( frame, 36 );

  OutputFrame::Ptr of;
  Vector3d pos;
  uint64_t r;
  pos.unpack(frame);
  r = frame->unpackInt64();

  IdSet oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);

  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();

  IdSet intersection;
  std::set_intersection( oblist.begin(), oblist.end(), visibleObjects.begin(), visibleObjects.end(),
      std::insert_iterator< IdSet >( intersection, intersection.begin() ) );
  
  temp_connection->sendSequence( frame, intersection.size() );

  IdSet::iterator obCurr = intersection.begin();
  for( ; obCurr != intersection.end(); ++obCurr) {
    ObjectView::Ptr object = player->getPlayerView()->getObjectView(*obCurr);
    if ( !object ){
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_Object, *obCurr));
        temp_connection->sendFail(frame, fec_NonExistant, "No Such Object", reflist);
    }else{
        OutputFrame::Ptr of = temp_connection->createFrame ( frame );
        try{
            object->packFrame( of, player->getId() );
            temp_connection->sendFrame ( of );
        }catch(FrameException e){
            temp_connection->sendFail(frame, e);
        }
    }
  }
}

void PlayerAgent::processGetObjectIds( InputFrame::Ptr frame ){
  DEBUG("Doing get object ids frame");
  OutputFrame::Ptr of = temp_connection->createFrame(frame);
  player->getPlayerView()->processGetObjectIds(frame, of);
  temp_connection->sendFrame(of);
}


void PlayerAgent::processGetObjectIdsByPos( InputFrame::Ptr frame ){
  DEBUG("doing get object ids by pos frame");
  OutputFrame::Ptr of = temp_connection->createFrame(frame);
  lengthCheckMin( frame, 36 );
  Vector3d pos;
  uint64_t r;

  pos.unpack(frame);
  r = frame->unpackInt64();

  IdSet oblist = Game::getGame()->getObjectManager()->getObjectsByPos(pos, r);
  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();
  IdSet intersection;
  std::set_intersection( oblist.begin(), oblist.end(), visibleObjects.begin(), visibleObjects.end(),
      std::insert_iterator< IdSet >( intersection, intersection.begin() ) );

  of->setType(ft03_ObjectIds_List);
  of->packInt(0);
  of->packInt(0);
  of->packInt(intersection.size());
  for(IdSet::iterator itcurr = intersection.begin(); itcurr != intersection.end(); ++itcurr){
    of->packInt(*itcurr);
    of->packInt64(Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime());
    Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
  }
  temp_connection->sendFrame(of);
}

void PlayerAgent::processGetObjectIdsByContainer( InputFrame::Ptr frame ){
  DEBUG("doing get object ids by container frame");
  lengthCheck( frame, 4 );
  uint32_t objectID = frame->unpackInt();
  IdSet visibleObjects = player->getPlayerView()->getVisibleObjects();
  RefList reflist;
  reflist.push_back(RefTypeAndId(rst_Object, objectID));
  if (visibleObjects.find(objectID) == visibleObjects.end()) {
    throw FrameException( fec_NonExistant, "No such Object", reflist );
  }
  IGObject::Ptr o = Game::getGame()->getObjectManager()->getObject(objectID);

  if (!o) {
    throw FrameException( fec_NonExistant, "No such Object", reflist );
  }
  IdSet contain = o->getContainedObjects();
  IdSet intersection;
  std::set_intersection( contain.begin(), contain.end(), visibleObjects.begin(), visibleObjects.end(),
      std::insert_iterator< IdSet >( intersection, intersection.begin() ) );

  IdModList modlist;
  for(IdSet::iterator itcurr = intersection.begin(); itcurr != intersection.end(); ++itcurr){
    modlist[*itcurr] = Game::getGame()->getObjectManager()->getObject(*itcurr)->getModTime();
    Game::getGame()->getObjectManager()->doneWithObject(*itcurr);
  }

  temp_connection->sendModList( frame, ft03_ObjectIds_List, 0, modlist, 0, 0, 0);
  Game::getGame()->getObjectManager()->doneWithObject(objectID);
}

void PlayerAgent::processGetObjectDesc( InputFrame::Ptr frame ){
  DEBUG("Doing get OrderDesc");
  versionCheck( frame, fv0_4 );
  int len = queryCheck( frame );

  // Object frames
  for ( int i=0 ; i < len; ++i ){
    uint32_t objecttype = frame->unpackInt();
    ObjectType* otype = Game::getGame()->getObjectTypeManager()->getObjectType(objecttype);
    if ( otype == NULL ){
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_ObjectType, objecttype));
        temp_connection->sendFail( frame, fec_NonExistant, "Object type does not exist", reflist);
    }else{
        try{
            temp_connection->send( frame, otype );
        }catch(FrameException e){
            temp_connection->sendFail(frame, e);
        }
    }
  }
}

void PlayerAgent::processGetObjectTypes( InputFrame::Ptr frame ){
  DEBUG("Doing get OrderTypes list");
  versionCheck( frame, fv0_4 );
  lengthCheck( frame, 20 );
  uint32_t lseqkey = frame->unpackInt();
  uint32_t seqkey = Game::getGame()->getObjectTypeManager()->getSeqKey();

  if(lseqkey == UINT32_NEG_ONE){
    //start new seqkey
    lseqkey = seqkey;
  }

  uint32_t start = frame->unpackInt();
  uint32_t num = frame->unpackInt();
  uint64_t fromtime = frame->unpackInt64();

  if(lseqkey != seqkey){
    throw FrameException( fec_TempUnavailable, "Invalid Sequence Key");
  }

  IdModList modlist = Game::getGame()->getObjectTypeManager()->getTypeModList(fromtime);
  
  temp_connection->sendModList( frame, ft04_ObjectTypes_List, lseqkey, modlist, num, start, fromtime);
}

void PlayerAgent::processGetOrder( InputFrame::Ptr frame ){
  DEBUG("Doing get order frame");
  lengthCheckMin( frame, 12 );
  uint32_t orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject::Ptr ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Object, orderqueueid));
      throw FrameException( fec_NonExistant, "No such Object", reflist);
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Object, orderqueueid));
      throw FrameException( fec_NonExistant, "No such Object OrderQueue", reflist);
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue::Ptr orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    throw FrameException( fec_NonExistant, "No such OrderQueue", reflist);
  }

  int num_orders = frame->unpackInt();

  lengthCheck( frame, 8 + 4 * num_orders );

  temp_connection->sendSequence(frame,num_orders);

  if(num_orders == 0){
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    throw FrameException( fec_FrameError,"No orders to get");
  }

  for(int i = 0; i < num_orders; i++){

    int ordpos = frame->unpackInt();
    Order *ord = orderqueue->getOrder(ordpos, player->getID());
    if (ord == NULL) {
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
      reflist.push_back(RefTypeAndId(rst_Order, ordpos));
      temp_connection->sendFail( frame, fec_TempUnavailable, "Could not get Order", reflist);
    }else{
        OutputFrame::Ptr of = temp_connection->createFrame(frame);
        ord->createFrame(of, ordpos);
        temp_connection->sendFrame(of);
    }
  }

}

void PlayerAgent::processGetOrderQueue( InputFrame::Ptr frame ){
  DEBUG("doing get orderqueue frame");
  if(frame->getDataLength() < 4){
    throw FrameException(fec_FrameError, "Invalid frame, too short");
  }
  int len = queryCheck(frame);
  
  for(int i = 0; i < len; i++){
    uint32_t oqid = frame->unpackInt();
    OrderQueue::Ptr oq = Game::getGame()->getOrderManager()->getOrderQueue(oqid);
    if(oq && (oq->isOwner(player->getID()) || oqid == 0)){
      OutputFrame::Ptr of = temp_connection->createFrame(frame);
      try{
        oq->pack(of);
        temp_connection->sendFrame(of);
      }catch(FrameException e){
        temp_connection->sendFail(frame, e);
      }
    }else{
      temp_connection->sendFail(frame, fec_NonExistant, "No such orderqueue");
    }
  }
}

void PlayerAgent::processGetOrderQueueIds( InputFrame::Ptr frame ){
  DEBUG("doing get orderqueue ids frame");
  if((frame->getDataLength() != 12 && frame->getVersion() == fv0_3) ||
     (frame->getDataLength() != 20 && frame->getVersion() >= fv0_4)){
    throw FrameException(fec_FrameError, "Invalid frame");
  }
  uint32_t seqnum = frame->unpackInt();
  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromserial = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromserial = frame->unpackInt64();
  }
  
  if(seqnum == UINT32_NEG_ONE){
    seqnum = 1;
  }
  
  //gen list
  IdModList oqids;
  
  std::map<uint32_t, OrderQueue::Ptr> orderqueues = Game::getGame()->getOrderManager()->getOrderQueues();

  for(std::map<uint32_t, OrderQueue::Ptr>::iterator itoq = orderqueues.begin(); itoq != orderqueues.end(); ++itoq){
    if(itoq->second->isOwner(player->getID()) || itoq->first == 0){
      if(fromserial == UINT64_NEG_ONE || itoq->second->getModTime() > fromserial){
        oqids[itoq->first] = itoq->second->getModTime();
      }
    }
  }

  if(snum > oqids.size()){
    throw FrameException(fec_NonExistant, "Start number too high");
  }

  if(numtoget > oqids.size() - snum){
    numtoget = oqids.size() - snum;
  }
 
  temp_connection->sendModList(frame, ft04_OrderQueueIds_List, seqnum, oqids, numtoget, snum, fromserial); 
}  

void PlayerAgent::processAddOrder( InputFrame::Ptr frame ){
  DEBUG("doing add order frame");
  if (frame->getDataLength() >= 8) {

    // See if we have a valid orderqueue id

    uint32_t orderqueueid = frame->unpackInt();

    if(frame->getVersion() <= fv0_3){
      IGObject::Ptr ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
      if(ob == NULL){
        throw FrameException( fec_NonExistant, "No such Object" );
      }
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
      if(oqop == NULL){
        throw FrameException( fec_NonExistant, "No such Object OrderQueue" );
      }
      orderqueueid = oqop->getQueueId();
    }

    OrderQueue::Ptr orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
    if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
      throw FrameException( fec_NonExistant, "No such Object Order Queue", reflist );
    }

    // Order Slot
    int pos = frame->unpackInt();

    // See if we have a valid order
    ordertypeid_t ordertype = frame->unpackInt();
    Order *ord = Game::getGame()->getOrderManager()->createOrder(ordertype);
    if (ord == NULL) {
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
      reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
      throw FrameException( fec_NonExistant, "No such order type", reflist );
    }
    
      ord->setOrderQueueId(orderqueueid);
      ord->inputFrame(frame, player->getID());

      if(orderqueue->addOrder(ord, pos, player->getID())) {
        OutputFrame::Ptr of = temp_connection->createFrame(frame);
        if(of->getVersion() >= fv0_4){
          ord->createFrame(of, pos);
        }else{
          of->setType(ft02_OK);
          of->packString("Order Added");
        }
        //update ObjectView
        uint32_t objid = orderqueue->getObjectId();
        if(objid != 0){
          player->getPlayerView()->getObjectView(objid)->touchModTime();
          player->getPlayerView()->updateObjectView(objid);
        }
        temp_connection->sendFrame(of);
      } else {
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
        reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
        throw FrameException( fec_TempUnavailable, "Not allowed to add that order type.", reflist );
      }
    
  } else {
    throw FrameException( fec_FrameError, "Invalid frame, Add Order, too short");
  }
}

void PlayerAgent::processRemoveOrder( InputFrame::Ptr frame ){
  DEBUG("doing remove order frame");

  lengthCheckMin( frame, 12 );

  int orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject::Ptr ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      throw FrameException( fec_NonExistant, "No such Object" );
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      throw FrameException( fec_NonExistant, "No such Object OrderQueue" );
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue::Ptr orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    throw FrameException( fec_NonExistant, "No such OrderQueue", reflist );
  }

  int num_orders = frame->unpackInt();

  lengthCheckMin( frame, 8 + 4 * num_orders  );

  temp_connection->sendSequence(frame,num_orders);

  for(int i = 0; i < num_orders; i++){
    int ordpos = frame->unpackInt();
    if (orderqueue->removeOrder(ordpos, player->getID())) {
      //update ObjectView
      uint32_t objid = orderqueue->getObjectId();
      if(objid != 0){
        player->getPlayerView()->getObjectView(objid)->touchModTime();
        player->getPlayerView()->updateObjectView(objid);
      }
      temp_connection->sendOK(frame, "Order removed");
    } else {
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
      reflist.push_back(RefTypeAndId(rst_Order, ordpos));
      temp_connection->sendFail( frame, fec_TempUnavailable, "Could not remove Order", reflist);
    }
  }
}


void PlayerAgent::processDescribeOrder( InputFrame::Ptr frame )
{
  DEBUG("doing describe order frame");

  int numdesc = queryCheck( frame );

  for(int i = 0; i < numdesc; i++){
    int ordertype = frame->unpackInt();
    OutputFrame::Ptr of = temp_connection->createFrame(frame);
    try{
        Game::getGame()->getOrderManager()->describeOrder(ordertype, of);
        temp_connection->sendFrame(of);
    }catch(FrameException e){
        temp_connection->sendFail(frame, e);
    }
  }
}

void PlayerAgent::processGetOrderTypes( InputFrame::Ptr frame ){
  DEBUG("doing get order types frame");

  versionCheck(frame,fv0_3);
  lengthCheck( frame, frame->getVersion() == fv0_3 ? 12 : 20 );

  OutputFrame::Ptr of = temp_connection->createFrame(frame);
  uint32_t lseqkey = frame->unpackInt();
  uint32_t start = frame->unpackInt();
  uint32_t num = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  if(lseqkey == UINT32_NEG_ONE){
    //start new seqkey
    lseqkey = Game::getGame()->getOrderManager()->getSeqKey();
  }

  if(lseqkey != Game::getGame()->getOrderManager()->getSeqKey()){
    throw FrameException(fec_TempUnavailable, "Invalid Sequence Key");
  }

  IdModList modlist = Game::getGame()->getOrderManager()->getModList(fromtime);

  temp_connection->sendModList( frame, ft03_OrderTypes_List, lseqkey, modlist, num, start, fromtime);
}

void PlayerAgent::processProbeOrder( InputFrame::Ptr frame ){
  DEBUG("doing probe order frame");

  versionCheck(frame,fv0_3);
  lengthCheckMin( frame, 8);

  int orderqueueid = frame->unpackInt();

  if(frame->getVersion() <= fv0_3){
    IGObject::Ptr ob = Game::getGame()->getObjectManager()->getObject(orderqueueid);
    if(ob == NULL){
      throw FrameException( fec_NonExistant, "No such Object" );
    }
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    if(oqop == NULL){
      throw FrameException( fec_NonExistant, "No such Object OrderQueue" );
    }
    orderqueueid = oqop->getQueueId();
  }

  OrderQueue::Ptr orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid);
  if(orderqueue == NULL || !orderqueue->isOwner(player->getID())){
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    throw FrameException( fec_NonExistant, "No such OrderQueue", reflist );
  }

  int pos = frame->unpackInt();

  // See if we have a valid order
  ordertypeid_t ordertype = frame->unpackInt();
  Order *ord = Game::getGame()->getOrderManager()->createOrder(ordertype);
  if (ord == NULL) {
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
    throw FrameException( fec_NonExistant, "No such Order type", reflist );
  }
  
  if(orderqueue->checkOrderType(ord->getType(), player->getID())){
    ord->setOrderQueueId(orderqueueid);
    OutputFrame::Ptr of = temp_connection->createFrame(frame);
    try {
      ord->inputFrame(frame, player->getID());
      ord->createFrame(of, pos);
      temp_connection->sendFrame(of);
    } catch ( FrameException& fe ) {
      DEBUG("Probe Order, could not unpack order");
      delete ord;
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
      reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
      throw FrameException( fec_Invalid, "Could not unpack order", reflist );
    }
  }else{
    delete ord;
    DEBUG("The order to be probed is not allowed on this object");
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderQueue, orderqueueid));
    reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
    throw FrameException( fec_PermUnavailable, "The order to be probed is not allowed on this object, try again", reflist);
  }
}

void PlayerAgent::processGetBoards( InputFrame::Ptr frame ){
  DEBUG("doing Get Boards frame");

  int numboards = queryCheck( frame );

  for(int i = 0; i < numboards; i++){
    uint32_t boardnum = frame->unpackInt();
    if(boardnum == 0 || boardnum == player->getBoardId()){
      Board::Ptr board = Game::getGame()->getBoardManager()->getBoard(player->getBoardId());
      temp_connection->send( frame, board );
    }else{
      temp_connection->sendFail( frame, fec_PermUnavailable, "No non-player boards yet");
    }
  }
}

void PlayerAgent::processGetBoardIds( InputFrame::Ptr frame ){
  DEBUG("Doing get board ids frame");

  versionCheck(frame,fv0_3);
  lengthCheck( frame, frame->getVersion() == fv0_3 ? 12 : 20 );

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

  OutputFrame::Ptr of = temp_connection->createFrame(frame);
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
  temp_connection->sendFrame(of);
}

void PlayerAgent::processGetMessages( InputFrame::Ptr frame ){
  DEBUG("doing Get Messages frame");

  lengthCheckMin( frame, 8 );

  int lboardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  lengthCheckMin( frame, 8 + 4 * nummsg );

  temp_connection->sendSequence(frame,nummsg);

  if(nummsg == 0){
    throw FrameException( fec_FrameError, "No messages to get");
  }

  Board::Ptr currboard;
  //HACK
  // should depend on what the player should be allowed to see
  if(lboardid == 0)
    lboardid = player->getBoardId();
  currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard.get() != NULL){
    for(int i = 0; i < nummsg; i++){
      int msgnum = frame->unpackInt();
      OutputFrame::Ptr of = temp_connection->createFrame(frame);

      currboard->packMessage(of, msgnum);

      temp_connection->sendFrame(of);

    }

  }else{
    throw FrameException( fec_NonExistant, "Board does not exist");
  }

}

void PlayerAgent::processPostMessage( InputFrame::Ptr frame ){
  DEBUG("doing Post Messages frame");


  lengthCheckMin( frame, 28 );

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
    msg->setSubject(frame->unpackString());
    msg->setBody(frame->unpackString());
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
    temp_connection->sendOK( frame, "Message posted" );
  }else{
    throw FrameException( fec_NonExistant, "Board does not exist");
  }
}

void PlayerAgent::processRemoveMessages( InputFrame::Ptr frame ){
  DEBUG("doing Remove Messages frame");

  lengthCheckMin( frame, 8 );

  int lboardid = frame->unpackInt();
  int nummsg = frame->unpackInt();

  lengthCheckMin( frame, 8 + 4 * nummsg );

  temp_connection->sendSequence(frame, nummsg);

  Board::Ptr currboard;
  //HACK
  // should depend on what the player should be allowed to see
  if(lboardid == 0)
    lboardid = player->getBoardId();
  currboard = Game::getGame()->getBoardManager()->getBoard(lboardid);

  if(currboard != NULL){
    for(int i = 0; i < nummsg; i++){
      int msgnum = frame->unpackInt();

      if(currboard->removeMessage(msgnum)){
        temp_connection->sendOK(frame, "Message removed");
      }else{
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_Board, lboardid));
        reflist.push_back(RefTypeAndId(rst_Message, msgnum));
        temp_connection->sendFail( frame, fec_NonExistant, "Message not removed, does exist", reflist);
      }
    }
  }else{
    throw FrameException( fec_NonExistant, "Board does not exist");
  }

}

void PlayerAgent::processGetResourceDescription( InputFrame::Ptr frame ){
  DEBUG("doing Get Resource Description frame");

  int numress = queryCheck( frame );

  for(int i = 0; i < numress; i++){
    int rnum = frame->unpackInt();

    const ResourceDescription::Ptr res = Game::getGame()->getResourceManager()->getResourceDescription(rnum);
    if(res != NULL){
      temp_connection->send(frame, res);
    }else{
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_ResouceType, rnum));
      temp_connection->sendFail( frame, fec_NonExistant, "No Resource Description available", reflist);
    }
  }
}

void PlayerAgent::processGetResourceTypes( InputFrame::Ptr frame ){
  DEBUG("doing Get Resource Types frame");

  versionCheck(frame,fv0_3);
  lengthCheck( frame, frame->getVersion() == fv0_3 ? 12 : 20 );

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

  ResourceManager::Ptr rm = Game::getGame()->getResourceManager();
  IdSet idset = rm->getAllIds();

  IdModList modlist;
  for(IdSet::iterator itcurr = idset.begin();
      itcurr != idset.end(); ++itcurr){
    const ResourceDescription::Ptr res = rm->getResourceDescription(*itcurr);
    if(fromtime == UINT64_NEG_ONE || res->getModTime() > fromtime){
      modlist[*itcurr] = res->getModTime();
    }
  }

  temp_connection->sendModList( frame, ft03_ResType_List, seqkey, modlist, numtoget, snum, fromtime);
}

void PlayerAgent::processGetPlayer( InputFrame::Ptr frame ){
  DEBUG("doing Get Player frame");

  int numplayers = queryCheck( frame );

  for(int i = 0; i < numplayers; i++){
    int pnum = frame->unpackInt();
    if(pnum == 0){
      temp_connection->send(frame,player);
    }else{
      if(pnum != -1){
        Player::Ptr p = Game::getGame()->getPlayerManager()->getPlayer(pnum);
        if(p != NULL){
          temp_connection->send(frame,p);
        }else{
          RefList reflist;
          reflist.push_back(RefTypeAndId(rst_Player, pnum));
          temp_connection->sendFail( frame, fec_NonExistant, "Player doesn't exist", reflist);
        }
      }else{
        temp_connection->sendFail( frame, fec_NonExistant, "Player -1 doesn't exist, invalid player id");
      }
    }
  }
}

void PlayerAgent::processGetPlayerIds( InputFrame::Ptr frame ){
  DEBUG("doing Get Resource Types frame");

  versionCheck(frame,fv0_4);
  lengthCheck( frame, 20 );

  uint32_t seqkey = frame->unpackInt();
  if(seqkey == UINT32_NEG_ONE){
    //start new seqkey
    seqkey = 0;
  }

  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;

  fromtime = frame->unpackInt64();

  PlayerManager::Ptr pm = Game::getGame()->getPlayerManager();
  IdSet idset = pm->getAllIds();

  IdModList modlist;
  for(IdSet::iterator itcurr = idset.begin();
      itcurr != idset.end(); ++itcurr){
    const Player::Ptr pl = pm->getPlayer(*itcurr);
    if(fromtime == UINT64_NEG_ONE || pl->getModTime() > fromtime){
      modlist[*itcurr] = pl->getModTime();
    }
  }

  temp_connection->sendModList( frame, ft04_PlayerIds_List, seqkey, modlist, numtoget, snum, fromtime);
}

void PlayerAgent::processGetCategory( InputFrame::Ptr frame ){
  DEBUG("doing Get Category frame");

  int numcats = queryCheck( frame );

  for(int i = 0; i < numcats; i++){
    int catnum = frame->unpackInt();
    Category::Ptr cat = Game::getGame()->getDesignStore()->getCategory(catnum);
    if(cat == NULL){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Category, catnum));
      temp_connection->sendFail( frame, fec_NonExistant, "No Such Category", reflist);
    }else{
      temp_connection->send(frame,cat);
    }
  }

}

void PlayerAgent::processGetCategoryIds( InputFrame::Ptr frame ){
  DEBUG("doing Get Category Ids frame");

  versionCheck(frame,fv0_3);
  lengthCheck( frame, frame->getVersion() == fv0_3 ? 12 : 20 );

  frame->unpackInt(); //seqnum
  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }


  DesignStore::Ptr ds = Game::getGame()->getDesignStore();
  IdSet cids = ds->getCategoryIds();

  IdModList modlist;
  for(IdSet::iterator itcurr = cids.begin();
      itcurr != cids.end(); ++itcurr){
    Category::Ptr cat = ds->getCategory(*itcurr);
    if(fromtime == UINT64_NEG_ONE || cat->getModTime() > fromtime){
      modlist[*itcurr] = cat->getModTime();
    }
  }

  temp_connection->sendModList( frame, ft03_CategoryIds_List, 0, modlist, numtoget, snum, fromtime);
}

void PlayerAgent::processGetDesign( InputFrame::Ptr frame ){
  DEBUG("doing Get Design frame");

  int numdesigns = queryCheck( frame );

  for(int i = 0; i < numdesigns; i++){
    int designnum = frame->unpackInt();
    DesignView::Ptr design = player->getPlayerView()->getDesignView( designnum );
    if (!design){
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_Design, designnum));
        temp_connection->sendFail(frame, fec_NonExistant, "No Such Design", reflist);
    }else{
        temp_connection->send( frame, design );
    }
  }
}

void PlayerAgent::processAddDesign( InputFrame::Ptr frame ){
  DEBUG("doing Add Design frame");

  lengthCheckMin( frame, 40 );

  Design::Ptr design( new Design() );
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
  design->setName(frame->unpackString());
  design->setDescription(frame->unpackString());
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(player->getID());
  uint32_t tpid = frame->unpackInt();
  if(player->getID() != tpid){
    DEBUG("Odd, client sent wrong player id %d", tpid);
  }
  design->setComponents( frame->unpackMap());
  //discard rest of frame

  DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  if(ds->addDesign(design)){
        DesignView::Ptr designv = player->getPlayerView()->getDesignView( design->getDesignId() );
        if (!designv) throw FrameException(fec_NonExistant, "No Such Design");
        temp_connection->send( frame, designv );
  }else{
    throw FrameException( fec_FrameError, "Could not add design");
  }
}

void PlayerAgent::processModifyDesign( InputFrame::Ptr frame ){
  DEBUG("doing Modify Design frame");

  lengthCheckMin( frame, 40 );

  Design::Ptr design( new Design() );
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
  design->setName(frame->unpackString());
  design->setDescription(frame->unpackString());
  frame->unpackInt(); //number in use, (client) read only
  design->setOwner(player->getID());
  uint32_t tpid = frame->unpackInt();
  if(player->getID() != tpid){
    DEBUG("Odd, client sent wrong player id %d", tpid);
  }
  design->setComponents(frame->unpackMap());
  //discard rest of frame

  DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  if(ds->modifyDesign(design)){
        DesignView::Ptr designv = player->getPlayerView()->getDesignView( design->getDesignId() );
        if (!designv){
            RefList reflist;
            reflist.push_back(RefTypeAndId(rst_Design, design->getDesignId()));
            throw FrameException(fec_NonExistant, "No Such Design", reflist);
        }
        temp_connection->send( frame, designv );
  }else{
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Design, design->getDesignId()));
    throw FrameException( fec_FrameError, "Could not modify design", reflist);
  }
}

void PlayerAgent::processGetDesignIds( InputFrame::Ptr frame ){
  OutputFrame::Ptr of = temp_connection->createFrame(frame);
  player->getPlayerView()->processGetDesignIds(frame, of);
  temp_connection->sendFrame(of);
}

void PlayerAgent::processGetComponent( InputFrame::Ptr frame ){
  DEBUG("doing Get Component frame");

  int numcomps = queryCheck( frame );

  for(int i = 0; i < numcomps; i++){
    int compnum = frame->unpackInt();
    ComponentView::Ptr comp = player->getPlayerView()->getComponentView(compnum);
    if (!comp){
        RefList reflist;
        reflist.push_back(RefTypeAndId(rst_Component, compnum));
        temp_connection->sendFail(frame, fec_NonExistant, "No Such Component", reflist);
    }else{
        temp_connection->send(frame, comp);
    }
  }
}

void PlayerAgent::processGetComponentIds( InputFrame::Ptr frame ){
  OutputFrame::Ptr of = temp_connection->createFrame(frame);
  player->getPlayerView()->processGetComponentIds(frame, of);

  temp_connection->sendFrame(of);
}

void PlayerAgent::processGetProperty( InputFrame::Ptr frame ){
  DEBUG("doing Get Property frame");

  DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  int numprops = queryCheck( frame );

  for(int i = 0; i < numprops; i++){
    int propnum = frame->unpackInt();
    Property::Ptr property = ds->getProperty(propnum);
    if(!property){
      RefList reflist;
      reflist.push_back(RefTypeAndId(rst_Property, propnum));
      temp_connection->sendFail(frame, fec_NonExistant, "No Such Property", reflist);
    }else{
      temp_connection->send(frame,property);
    }
  }
}

void PlayerAgent::processGetPropertyIds( InputFrame::Ptr frame ){
  DEBUG("doing Get Property Ids frame");

  versionCheck( frame, fv0_3 );
  lengthCheck( frame, frame->getVersion() == fv0_3 ? 12 : 20 );

  IdSet propids = Game::getGame()->getDesignStore()->getPropertyIds();
  frame->unpackInt(); //seqnum
  uint32_t snum = frame->unpackInt();
  uint32_t numtoget = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  IdModList modlist;
  for(IdSet::iterator itcurr = propids.begin();
      itcurr != propids.end(); ++itcurr){
    Property::Ptr prop = ds->getProperty(*itcurr);
    if(fromtime == UINT64_NEG_ONE || prop->getModTime() > fromtime){
      modlist[*itcurr] = prop->getModTime();
    }
  }

  temp_connection->sendModList(frame,ft03_PropertyIds_List,0,modlist,numtoget,snum,fromtime);
}

void PlayerAgent::processTurnFinished( InputFrame::Ptr frame ){
  DEBUG("doing Done Turn frame");

  if(frame->getVersion() < fv0_4){
    DEBUG("Turn Finished Frame: protocol version not high enough, but continuing anyway");
    //     throw FrameException(fec_FrameError, "Finished Turn frame isn't supported in this protocol");
  }

  Game::getGame()->getTurnTimer()->playerFinishedTurn(player->getID());

  temp_connection->sendOK(frame, "Thanks for letting me know you have finished your turn.");

}

void PlayerAgent::versionCheck( InputFrame::Ptr frame, ProtocolVersion min_version )
{
  if(frame->getVersion() < min_version ){
    DEBUG("Frame protocol version not high enough");
    throw FrameException( fec_FrameError, "This feature isn't supported on this protocol version");
  }
}

void PlayerAgent::lengthCheck( InputFrame::Ptr frame, uint32_t length )
{
  if ( frame->getDataLength() != (int)length ) {
    DEBUG("Frame of invalid size");
    throw FrameException( fec_FrameError, "Frame is of invalid size");
  }
}

void PlayerAgent::lengthCheckMin( InputFrame::Ptr frame, uint32_t length )
{
  if ( frame->getDataLength() < (int)length ) {
    DEBUG("Frame is too short");
    throw FrameException( fec_FrameError, "Frame is too short");
  }
}

int PlayerAgent::queryCheck( InputFrame::Ptr frame )
{
  lengthCheckMin( frame, 4 );

  int result = frame->unpackInt();
  if ( result <= 0 ) {
    DEBUG( "Asked for no data, silly client... " );
    throw FrameException( fec_NonExistant, "You didn't ask for any data, try again");
  }

  lengthCheckMin( frame, 4 + 4 * result );
  temp_connection->sendSequence( frame, result );

  return result;
}
