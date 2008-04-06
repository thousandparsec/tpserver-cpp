#ifndef MYSQLPERSISTENCE_H
#define MYSQLPERSISTENCE_H
/*  Mysql Persistence class
 *
 *  Copyright (C) 2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <map>

#include <tpserver/persistence.h>

typedef struct st_mysql MYSQL;
class SpaceCoordParam;
class ObjectOrderParameter;
class ListParameter;
class StringParameter;
class TimeParameter;

class Position3dObjectParam;
class Velocity3dObjectParam;
class OrderQueueObjectParam;
class ResourceListObjectParam;
class ReferenceObjectParam;

class MysqlPersistence : public Persistence{
public:
    MysqlPersistence();
    virtual ~MysqlPersistence();

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
    
    virtual bool saveObjectView(uint32_t playerid, ObjectView* ov);
    virtual ObjectView* retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn = 0xffffffff);
    
    virtual bool saveDesignView(uint32_t playerid, DesignView* dv);
    virtual DesignView* retrieveDesignView(uint32_t playerid, uint32_t designid);
    
    virtual bool saveComponentView(uint32_t playerid, ComponentView* cv);
    virtual ComponentView* retrieveComponentView(uint32_t playerid, uint32_t componentid);

    std::string addslashes(const std::string& in) const;
    uint32_t getTableVersion(const std::string& name);

private:
  
  bool updateSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp);
  bool retrieveSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp);
  bool removeSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos);
  bool updateListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp);
  bool retrieveListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp);
  bool removeListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos);
  bool updateObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob);
  bool retrieveObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob);
  bool removeObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos);
  bool updateStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st);
  bool retrieveStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st);
  bool removeStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos);
  bool updateTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp);
  bool retrieveTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp);
  bool removeTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos);
  
  bool updatePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob);
  bool retrievePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob);
  bool updateVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob);
  bool retrieveVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob);
  bool updateOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob);
  bool retrieveOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob);
  bool updateResourceListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ResourceListObjectParam* rob);
  bool retrieveResourceListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ResourceListObjectParam* rob);
  bool updateReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob);
  bool retrieveReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob);
  
    void lock();
    void unlock();
    MYSQL *conn;

};

#endif
