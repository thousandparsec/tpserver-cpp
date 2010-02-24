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
class RefQuantityListObjectParam;
class IntegerObjectParam;
class SizeObjectParam;
class MediaObjectParam;

class MysqlPersistence : public Persistence{
public:
    MysqlPersistence();
    virtual ~MysqlPersistence();

    virtual bool init();
    virtual void shutdown();

    virtual bool saveGameInfo();
    virtual bool retrieveGameInfo();

    virtual bool saveObject(boost::shared_ptr<IGObject> ob);
    virtual boost::shared_ptr<IGObject> retrieveObject(uint32_t obid);
    virtual uint32_t getMaxObjectId();
    virtual IdSet getObjectIds();

    virtual bool saveOrderQueue(const boost::shared_ptr<OrderQueue> oq);
    virtual bool updateOrderQueue(const boost::shared_ptr<OrderQueue> oq);
    virtual boost::shared_ptr<OrderQueue> retrieveOrderQueue(uint32_t oqid);
    virtual bool removeOrderQueue(uint32_t oqid);
    virtual IdSet getOrderQueueIds();
    virtual uint32_t getMaxOrderQueueId();
    
    virtual bool saveOrder(uint32_t queueid, uint32_t ordid, Order* ord);
    virtual bool updateOrder(uint32_t queueid, uint32_t ordid, Order* ord);
    virtual Order* retrieveOrder(uint32_t queueid, uint32_t ordid);
    virtual bool removeOrder(uint32_t queueid, uint32_t ordid);
    

    virtual bool saveBoard(boost::shared_ptr<Board> board);
    virtual bool updateBoard(boost::shared_ptr<Board> board);
    virtual boost::shared_ptr<Board> retrieveBoard(uint32_t boardid);
    virtual uint32_t getMaxBoardId();
    virtual IdSet getBoardIds();

    virtual bool saveMessage( boost::shared_ptr< Message > msg);
    virtual boost::shared_ptr< Message > retrieveMessage(uint32_t msgid);
    virtual bool removeMessage(uint32_t msgid);
    virtual bool saveMessageList(uint32_t bid, std::list<uint32_t> list);
    virtual std::list<uint32_t> retrieveMessageList(uint32_t bid);
    virtual uint32_t getMaxMessageId();

    virtual bool saveResource(boost::shared_ptr<ResourceDescription> res);
    virtual boost::shared_ptr<ResourceDescription> retrieveResource(uint32_t restype);
    virtual uint32_t getMaxResourceId();
    virtual IdSet getResourceIds();

    virtual bool savePlayer(boost::shared_ptr<Player> player);
    virtual bool updatePlayer(boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<Player> retrievePlayer(uint32_t playerid);
    virtual uint32_t getMaxPlayerId();
    virtual IdSet getPlayerIds();

    virtual bool saveCategory(boost::shared_ptr<Category> cat);
    virtual boost::shared_ptr<Category> retrieveCategory(uint32_t catid);
    virtual uint32_t getMaxCategoryId();
    virtual IdSet getCategoryIds();

    virtual bool saveDesign(boost::shared_ptr<Design> design);
    virtual bool updateDesign(boost::shared_ptr<Design> design);
    virtual boost::shared_ptr<Design> retrieveDesign(uint32_t designid);
    virtual uint32_t getMaxDesignId();
    virtual IdSet getDesignIds();

    virtual bool saveComponent(boost::shared_ptr<Component> comp);
    virtual boost::shared_ptr<Component> retrieveComponent(uint32_t compid);
    virtual uint32_t getMaxComponentId();
    virtual IdSet getComponentIds();

    virtual bool saveProperty(boost::shared_ptr<Property> prop);
    virtual boost::shared_ptr<Property> retrieveProperty(uint32_t propid);
    virtual uint32_t getMaxPropertyId();
    virtual IdSet getPropertyIds();
    
    virtual bool saveObjectView(uint32_t playerid, boost::shared_ptr<ObjectView> ov);
    virtual boost::shared_ptr<ObjectView> retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn = 0xffffffff);
    
    virtual bool saveDesignView(uint32_t playerid, boost::shared_ptr<DesignView> dv);
    virtual boost::shared_ptr<DesignView> retrieveDesignView(uint32_t playerid, uint32_t designid);
    
    virtual bool saveComponentView(uint32_t playerid, boost::shared_ptr<ComponentView> cv);
    virtual boost::shared_ptr<ComponentView> retrieveComponentView(uint32_t playerid, uint32_t componentid);

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
  bool updateRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob);
  bool retrieveRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob);
  bool updateIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob);
  bool retrieveIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob);
  bool updateSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob);
  bool retrieveSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob);
  bool updateMediaObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, MediaObjectParam* mob);
  bool retrieveMediaObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, MediaObjectParam* mob);
  
    void lock();
    void unlock();
    MYSQL *conn;

  void idSetToStream( std::ostringstream& stream, const uint32_t id, const IdSet& idset ) const;
  void idSetToStream( std::ostringstream& stream, const uint32_t id, const uint32_t id2, const IdSet& idset ) const;
  void idListToStream( std::ostringstream& stream, const uint32_t id, const IdList& idlist ) const;
  void idMapToStream( std::ostringstream& stream, const uint32_t id, const IdMap& idmap ) const;
  void idMapToStream( std::ostringstream& stream, const uint32_t id, const uint32_t id2, const IdMap& idmap ) const;

  void insertSet ( const std::string& table, uint32_t id, const IdSet& idset );
  void insertSet ( const std::string& table, uint32_t id, uint32_t id2, const IdSet& idset );
  void insertList( const std::string& table, uint32_t id, const IdList& idlist );
  void insertMap ( const std::string& table, uint32_t id, const IdMap& idmap );
  void insertMap ( const std::string& table, uint32_t id, uint32_t id2, const IdMap& idmap );

  void singleQuery( const std::string& query );
  uint32_t valueQuery( const std::string& query );

  const IdSet idSetQuery( const std::string& query );
  const IdList idListQuery( const std::string& query );
  const IdMap idMapQuery( const std::string& query );
};

class MysqlQuery {
  public:
    MysqlQuery( MYSQL *conn, const std::string& new_query );
    ~MysqlQuery();
       
    static void lock() {}
    static void unlock() {}

    const std::string get( uint32_t index );
    int getInt( uint32_t index );
    uint64_t getU64( uint32_t index );
    bool validRow();
    bool nextRow();

  private:

    void fetchResult();
    MYSQL *connection;
    MYSQL_RES* result;
    MYSQL_ROW row;
    
    std::string query;

    MysqlQuery() {};
};

#endif
