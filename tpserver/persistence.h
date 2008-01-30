#ifndef PERSISTENCE_H
#define PERSISTENCE_H
/*  Persistence class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <string>
#include <list>
#include <set>

class IGObject;
class Order;
class OrderQueue;
class Board;
class Message;
class ResourceDescription;
class Player;
class Category;
class Design;
class Component;
class Property;

class Persistence{
public:
    virtual ~Persistence();

    virtual bool init();
    virtual void shutdown();
    
    virtual bool saveGameInfo();
    virtual bool retrieveGameInfo();

    virtual bool saveObject(IGObject* ob);
    virtual bool updateObject(IGObject* ob);
    virtual IGObject* retrieveObject(uint32_t obid);
    virtual bool removeObject(uint32_t obid);
    virtual uint32_t getMaxObjectId();
    virtual std::set<uint32_t> getObjectIds();

    virtual bool saveOrderQueue(const OrderQueue* oq);
    virtual bool updateOrderQueue(const OrderQueue* oq);
    virtual OrderQueue* retrieveOrderQueue(uint32_t oqid);
    virtual bool removeOrderQueue(uint32_t oqid);
    virtual std::set<uint32_t> getOrderQueueIds();
    virtual uint32_t getMaxOrderQueueId();
    
    virtual bool saveOrder(uint32_t queueid, uint32_t ordid, Order* ord);
    virtual bool updateOrder(uint32_t queueid, uint32_t ordid, Order* ord);
    virtual Order* retrieveOrder(uint32_t queueid, uint32_t ordid);
    virtual bool removeOrder(uint32_t queueid, uint32_t ordid);
    

    virtual bool saveBoard(Board* board);
    virtual bool updateBoard(Board* board);
    virtual Board* retrieveBoard(uint32_t boardid);
    virtual uint32_t getMaxBoardId();
    virtual std::set<uint32_t> getBoardIds();

    virtual bool saveMessage(uint32_t msgid, Message* msg);
    virtual Message* retrieveMessage(uint32_t msgid);
    virtual bool removeMessage(uint32_t msgid);
    virtual bool saveMessageList(uint32_t bid, std::list<uint32_t> list);
    virtual std::list<uint32_t> retrieveMessageList(uint32_t bid);
    virtual uint32_t getMaxMessageId();
    
    virtual bool saveResource(ResourceDescription* res);
    virtual ResourceDescription* retrieveResource(uint32_t restype);
    virtual uint32_t getMaxResourceId();
    virtual std::set<uint32_t> getResourceIds();

    virtual bool savePlayer(Player* player);
    virtual bool updatePlayer(Player* player);
    virtual Player* retrievePlayer(uint32_t playerid);
    virtual uint32_t getMaxPlayerId();
    virtual std::set<uint32_t> getPlayerIds();

    virtual bool saveCategory(Category* cat);
    virtual Category* retrieveCategory(uint32_t catid);
    virtual uint32_t getMaxCategoryId();
    virtual std::set<uint32_t> getCategoryIds();

    virtual bool saveDesign(Design* design);
    virtual bool updateDesign(Design* design);
    virtual Design* retrieveDesign(uint32_t designid);
    virtual uint32_t getMaxDesignId();
    virtual std::set<uint32_t> getDesignIds();

    virtual bool saveComponent(Component* comp);
    virtual Component* retrieveComponent(uint32_t compid);
    virtual uint32_t getMaxComponentId();
    virtual std::set<uint32_t> getComponentIds();

    virtual bool saveProperty(Property* prop);
    virtual Property* retrieveProperty(uint32_t propid);
    virtual uint32_t getMaxPropertyId();
    virtual std::set<uint32_t> getPropertyIds();

};

#endif
