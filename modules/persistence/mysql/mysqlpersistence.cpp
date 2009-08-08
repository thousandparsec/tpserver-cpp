/*  Mysql persistence class
 *
 *  Copyright (C) 2005,2006,2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <sstream>

#include <tpserver/logging.h>
#include <tpserver/settings.h>
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objecttypemanager.h>

#include <tpserver/object.h>
#include <tpserver/orderqueue.h>
#include <tpserver/order.h>
#include <tpserver/orderparameter.h>
#include <tpserver/orderparameters.h>
#include <tpserver/board.h>
#include <tpserver/message.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/category.h>
#include <tpserver/design.h>
#include <tpserver/propertyvalue.h>
#include <tpserver/component.h>
#include <tpserver/property.h>

#include <tpserver/objectview.h>
#include <tpserver/designview.h>
#include <tpserver/componentview.h>

//Objectparameters
#include <tpserver/position3dobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/sizeobjectparam.h>

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

#include "mysqlpersistence.h"

extern "C" {
#define tp_init libtpmysql_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setPersistence(new MysqlPersistence());
  }
}

class MysqlException : public std::exception {
  public:
    MysqlException( MYSQL* conn, const std::string& error ) {
      errorstr = error + " " + std::string( mysql_error(conn) );
      Logger::getLogger()->error( "MySQL : "+errorstr );
    }
    const char* what() const throws() {
      return errorstr.c_str();
    }
  private:
    std::string errorstr;
}

MysqlPersistence::MysqlPersistence() : conn(NULL){
}

  MysqlPersistence::~MysqlPersistence(){
    if(conn != NULL)
      shutdown();
  }

bool MysqlPersistence::init(){
  lock();
  if(conn != NULL){
    Logger::getLogger()->warning("Mysql Persistence already running");
    unlock();
    return false;
  }
  conn = mysql_init(NULL);
  if(conn == NULL){
    Logger::getLogger()->error("Could not init mysql");
    unlock();
    return false;
  }
  Settings* conf = Settings::getSettings();

  const char* host = NULL;
  std::string shost = conf->get("mysql_host");
  if(shost.length() != 0)
    host = shost.c_str();
  const char* user = NULL;
  std::string suser = conf->get("mysql_user");
  if(suser.length() != 0)
    user = suser.c_str();
  const char* pass = NULL;
  std::string spass = conf->get("mysql_pass");
  if(spass.length() != 0)
    pass = spass.c_str();
  const char* db = NULL;
  std::string sdb = conf->get("mysql_db");
  if(sdb.length() != 0)
    db = sdb.c_str();
  else{
    Logger::getLogger()->error("mysql database name not specified");
    mysql_close(conn);
    conn = NULL;
    unlock();
    return false;
  }
  const char* sock = NULL;
  std::string ssock = conf->get("mysql_socket");
  if(ssock.length() != 0)
    sock = ssock.c_str();
  int port = atoi(conf->get("mysql_port").c_str());
  if(mysql_real_connect(conn, host, user, pass, db, port, sock, CLIENT_FOUND_ROWS) == NULL){
    Logger::getLogger()->error("Could not connect to mysql server - %s", mysql_error(conn));
    mysql_close(conn);
    conn = NULL;
    unlock();
    return false;
  }

  // check for tables, create if necessary

  if(mysql_query(conn, "SELECT * FROM tableversion;") != 0){
    // create tables
    Logger::getLogger()->info("Creating database tables");
    try{
      if(mysql_query(conn, "CREATE TABLE tableversion ( tableid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
            "name VARCHAR(50) NOT NULL UNIQUE, version INT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "INSERT INTO tableversion VALUES (NULL, 'tableversion', 1), (NULL, 'gameinfo', 1), "
            "(NULL, 'object', 0), (NULL, 'objectparamposition', 0), (NULL, 'objectparamvelocity', 0), "
            "(NULL, 'objectparamorderqueue', 0), (NULL, 'objectparamresourcelist', 0), "
            "(NULL, 'objectparamreference', 0), (NULL, 'objectparamrefquantitylist', 0), "
            "(NULL, 'objectparaminteger', 0), (NULL, 'objectparamsize', 0), "
            "(NULL, 'orderqueue', 0), (NULL, 'orderqueueowner', 0), (NULL, 'orderqueueallowedtype', 0), "
            "(NULL, 'ordertype', 0), (NULL, 'orderresource', 0), (NULL, 'orderslot', 0), "
            "(NULL, 'orderparamspace', 0), (NULL, 'orderparamobject', 0), "
            "(NULL, 'orderparamstring', 0), (NULL, 'orderparamtime', 0), "
            "(NULL, 'orderparamlist', 0), "
            "(NULL, 'board', 0), (NULL, 'message', 0), (NULL, 'messagereference', 0), (NULL, 'messageslot', 0), "
            "(NULL, 'player', 0), (NULL, 'playerscore', 0), (NULL, 'playerdesignview', 0), (NULL, 'playerdesignviewcomp', 0), "
            "(NULL, 'playerdesignviewprop', 0), (NULL, 'playerdesignusable', 0), (NULL, 'playercomponentview', 0), "
            "(NULL, 'playercomponentviewcat', 0), (NULL, 'playercomponentviewproperty', 0), "
            "(NULL, 'playercomponentusable', 0), (NULL, 'playerobjectview', 0), (NULL, 'playerobjectowned', 0), "
            "(NULL, 'category', 0), (NULL, 'design',0), (NULL, 'designcomponent', 0), (NULL, 'designproperty', 0), "
            "(NULL, 'component', 0), (NULL, 'componentcat', 0), (NULL, 'componentproperty', 0), "
            "(NULL, 'property', 0), (NULL, 'propertycat', 0), (NULL, 'resourcedesc', 0);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE gameinfo (metakey VARCHAR(50) NOT NULL, ctime BIGINT UNSIGNED NOT NULL PRIMARY KEY, turnnum INT UNSIGNED NOT NULL, turnname VARCHAR(50) NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE object (objectid INT UNSIGNED NOT NULL, turnnum INT UNSIGNED NOT NULL, alive TINYINT UNSIGNED NOT NULL, type INT UNSIGNED NOT NULL, " 
            "name TEXT NOT NULL, description TEXT NOT NULL, parentid INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turnnum));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamposition (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, posx BIGINT NOT NULL, posy BIGINT NOT NULL, posz BIGINT NOT NULL, relative INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamvelocity (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, velx BIGINT NOT NULL, vely BIGINT NOT NULL, velz BIGINT NOT NULL, relative INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamorderqueue (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, queueid INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamresourcelist (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, resid INT UNSIGNED NOT NULL, available INT UNSIGNED NOT NULL, possible INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos, resid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamreference (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, reftype INT NOT NULL, refval INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamrefquantitylist (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, reftype INT NOT NULL, refid INT UNSIGNED NOT NULL, quant INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos, reftype, refid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparaminteger (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, val INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE objectparamsize (objectid INT UNSIGNED NOT NULL, turn INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, paramgroupid INT UNSIGNED NOT NULL, paramgrouppos INT UNSIGNED NOT NULL, size BIGINT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turn, playerid, paramgroupid, paramgrouppos));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderqueue (queueid INT UNSIGNED NOT NULL PRIMARY KEY, objectid INT UNSIGNED NOT NULL, active TINYINT NOT NULL, repeating TINYINT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderqueueowner (queueid INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL, PRIMARY KEY(queueid, playerid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderqueueallowedtype (queueid INT UNSIGNED NOT NULL, ordertype INT UNSIGNED NOT NULL, PRIMARY KEY(queueid, ordertype));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE ordertype (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, type INT UNSIGNED NOT NULL, turns INT UNSIGNED NOT NULL, PRIMARY KEY(queueid, orderid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderresource (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, resourceid INT UNSIGNED NOT NULL, amount INT UNSIGNED NOT NULL, PRIMARY KEY (queueid, orderid, resourceid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderparamspace (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, position INT UNSIGNED NOT NULL, posx BIGINT NOT NULL, posy BIGINT NOT NULL, posz BIGINT NOT NULL, PRIMARY KEY (queueid, orderid, position));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderparamobject (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, position INT UNSIGNED NOT NULL, objectid INT UNSIGNED NOT NULL, PRIMARY KEY (queueid, orderid, position));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderparamstring (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, position INT UNSIGNED NOT NULL, thestring TEXT NOT NULL, PRIMARY KEY (queueid, orderid, position));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderparamtime (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, position INT UNSIGNED NOT NULL, turns INT UNSIGNED NOT NULL, PRIMARY KEY (queueid, orderid, position));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderparamlist (queueid INT UNSIGNED NOT NULL, orderid INT UNSIGNED NOT NULL, position INT UNSIGNED NOT NULL, listid INT UNSIGNED NOT NULL, amount INT UNSIGNED NOT NULL, PRIMARY KEY (queueid, orderid, position, listid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE orderslot (queueid INT UNSIGNED NOT NULL, slot INT UNSIGNED NOT NULL, "
            "orderid INT UNSIGNED NOT NULL, PRIMARY KEY (queueid, slot, orderid), UNIQUE (queueid, slot), UNIQUE(queueid, orderid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE board (boardid INT UNSIGNED NOT NULL PRIMARY KEY, "
            "name TEXT NOT NULL, description TEXT NOT NULL, nummessages INT UNSIGNED NOT NULL, "
            "modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE message (messageid INT UNSIGNED NOT NULL PRIMARY KEY, subject TEXT NOT NULL, "
            "body TEXT NOT NULL, turn INT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE messagereference (messageid INT UNSIGNED NOT NULL, type INT NOT NULL, "
            "refid INT UNSIGNED NOT NULL, PRIMARY KEY (messageid, type, refid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE messageslot (boardid INT UNSIGNED NOT NULL, slot INT UNSIGNED NOT NULL, "
            "messageid INT UNSIGNED NOT NULL UNIQUE, PRIMARY KEY (boardid, slot));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE player (playerid INT UNSIGNED NOT NULL PRIMARY KEY, name TEXT NOT NULL, "
            "password TEXT NOT NULL, email TEXT NOT NULL, comment TEXT NOT NULL, boardid INT UNSIGNED NOT NULL, "
            "alive TINYINT UNSIGNED NOT NULL, modtime BIGINT NOT NULL, UNIQUE (name(255)));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerscore (playerid INT UNSIGNED NOT NULL, scoreid INT UNSIGNED NOT NULL, scoreval INT UNSIGNED NOT NULL, PRIMARY KEY(playerid, scoreid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerdesignview (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
            "completelyvisible TINYINT NOT NULL, namevisible TINYINT NOT NULL,"
            "visname TEXT NOT NULL, descvisible TINYINT NOT NULL, visdesc TEXT NOT NULL, existvisible TINYINT NOT NULL, visexist INT UNSIGNED NOT NULL, ownervisible TINYINT NOT NULL, visowner INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY (playerid, designid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerdesignviewcomp (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
            "componentid INT UNSIGNED NOT NULL, quantity INT UNSIGNED NOT NULL, "
            "PRIMARY KEY (playerid, designid, componentid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerdesignviewprop (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
            "propertyid INT UNSIGNED NOT NULL, value TEXT NOT NULL, "
            "PRIMARY KEY (playerid, designid, propertyid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerdesignusable (playerid INT UNSIGNED NOT NULL, designid INT UNSIGNED NOT NULL, "
            "PRIMARY KEY (playerid, designid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playercomponentview (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
            "completelyvisible TINYINT NOT NULL, namevisible TINYINT NOT NULL, visname TEXT NOT NULL, "
            "descvisible TINYINT NOT NULL, visdesc TEXT NOT NULL, reqfuncvis TINYINT NOT NULL, "
            "modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY (playerid, componentid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playercomponentviewcat (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
            "categoryid INT UNSIGNED NOT NULL, PRIMARY KEY (playerid, componentid, categoryid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playercomponentviewproperty (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
            "propertyid INT UNSIGNED NOT NULL, PRIMARY KEY (playerid, componentid, propertyid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playercomponentusable (playerid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
            "PRIMARY KEY (playerid, componentid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerobjectview (playerid INT UNSIGNED NOT NULL, objectid INT UNSIGNED NOT NULL, "
            "turnnum INT UNSIGNED NOT NULL, completelyvisible TINYINT NOT NULL, gone TINYINT NOT NULL, "
            "namevisible TINYINT NOT NULL, visname TEXT NOT NULL, descvisible TINYINT NOT NULL, "
            "visdesc TEXT NOT NULL, "
            "modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY (playerid, objectid, turnnum));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE playerobjectowned (playerid INT UNSIGNED NOT NULL, objectid INT UNSIGNED NOT NULL, "
            "PRIMARY KEY (playerid, objectid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE category (categoryid INT UNSIGNED NOT NULL PRIMARY KEY, name TEXT NOT NULL, "
            "description TEXT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE design (designid INT UNSIGNED NOT NULL PRIMARY KEY, categoryid INT UNSIGNED NOT NULL,"
            "name TEXT NOT NULL, description TEXT NOT NULL, owner INT NOT NULL, inuse INT UNSIGNED NOT NULL,"
            "numexist INT UNSIGNED NOT NULL, valid TINYINT NOT NULL, feedback TEXT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE designcomponent (designid INT UNSIGNED NOT NULL, componentid INT UNSIGNED NOT NULL, "
            "count INT UNSIGNED NOT NULL, PRIMARY KEY (designid, componentid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE designproperty (designid INT UNSIGNED NOT NULL, propertyid INT UNSIGNED NOT NULL, "
            "value DOUBLE  NOT NULL, displaystring TEXT NOT NULL, PRIMARY KEY (designid, propertyid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE component (componentid INT UNSIGNED NOT NULL PRIMARY KEY, "
            "name TEXT NOT NULL, description TEXT NOT NULL, tpclrequiresfunc TEXT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE componentcat (componentid INT UNSIGNED NOT NULL, categoryid INT UNSIGNED NOT NULL, PRIMARY KEY (componentid, categoryid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE componentproperty (componentid INT UNSIGNED NOT NULL, propertyid INT UNSIGNED NOT NULL, "
            "tpclvaluefunc TEXT NOT NULL, PRIMARY KEY (componentid, propertyid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE property (propertyid INT UNSIGNED NOT NULL PRIMARY KEY,"
            "rank INT UNSIGNED NOT NULL, name TEXT NOT NULL, displayname TEXT NOT NULL, description TEXT NOT NULL, "
            "tpcldisplayfunc TEXT NOT NULL, tpclrequiresfunc TEXT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE propertycat (propertyid INT UNSIGNED NOT NULL, categoryid INT UNSIGNED NOT NULL, PRIMARY KEY (propertyid, categoryid));") != 0){
        throw std::exception();
      }
      if(mysql_query(conn, "CREATE TABLE resourcedesc (resourcetype INT UNSIGNED NOT NULL PRIMARY KEY, name_sig TEXT NOT NULL,"
            "name_plur TEXT NOT NULL, unit_sig TEXT NOT NULL, uint_plur TEXT NOT NULL, description TEXT NOT NULL, "
            "mass INT UNSIGNED NOT NULL, volume INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
        throw std::exception();
      }
    }catch(std::exception e){
      Logger::getLogger()->error("Mysql creating tables: %s", mysql_error(conn));
      Logger::getLogger()->error("You may need to delete the tables and start again");
      mysql_close(conn);
      conn = NULL;
      unlock();
      return false;
    }
  }else{
    // check for tables to be updated.
    MYSQL_RES *tableversions = mysql_store_result(conn);
    if(tableversions == NULL){
      Logger::getLogger()->error("Mysql: table versions query result error: %s", mysql_error(conn));
      Logger::getLogger()->error("You may need to delete the tables and start again");
      mysql_close(conn);
      conn = NULL;
      unlock();
      return false;
    }
    //Possible central table updates
    mysql_free_result(tableversions);

    try{
      if(getTableVersion("tableversion") == 0){
        Logger::getLogger()->error("Old database format detected.");
        Logger::getLogger()->error("Incompatable old table formats and missing tables detected.");
        Logger::getLogger()->error("Changes to most stored classes means there is no way to update from your current database to the newer format");
        Logger::getLogger()->error("I cannot stress this enough: Please shutdown your game, delete the contents of the database and start again. Sorry");
        Logger::getLogger()->error("Mysql persistence NOT STARTED");
        return false;
      }
      if(getTableVersion("gameinfo") == 0){
        if(mysql_query(conn, "ALTER TABLE gameinfo ADD COLUMN turnname VARCHAR(50) NOT NULL;") != 0){
          Logger::getLogger()->error("Can't alter gameinfo table, please reset the database");
          return false;
        }
      }
    }catch(std::exception e){
    }

  }

  unlock();

  if(mysql_thread_safe()){
    Logger::getLogger()->debug("Mysql is thread safe");
  }else{
    Logger::getLogger()->debug("Mysql is NOT thread safe");
  }

  return true;
}

void MysqlPersistence::shutdown(){
  lock();
  // TEMP HACK
  //     mysql_query(conn, "DELETE FROM object;");
  //     mysql_query(conn, "DELETE FROM universe;");
  //     mysql_query(conn, "DELETE FROM planet;");
  //     mysql_query(conn, "DELETE FROM fleet;");
  //     mysql_query(conn, "DELETE FROM fleetship;");
  //     mysql_query(conn, "DELETE FROM ordertype;");
  //     mysql_query(conn, "DELETE FROM orderslot;");
  //     mysql_query(conn, "DELETE FROM player;");
  //     mysql_query(conn, "DELETE FROM playerdesignvisible;");
  //     mysql_query(conn, "DELETE FROM playerdesignusable;");
  //     mysql_query(conn, "DELETE FROM playercomponentvisible;");
  //     mysql_query(conn, "DELETE FROM playercomponentusable;");
  //     mysql_query(conn, "DELETE FROM playerobjectvisible;");
  //     mysql_query(conn, "DELETE FROM category;");
  //     mysql_query(conn, "DELETE FROM design;");
  //     mysql_query(conn, "DELETE FROM designcomponent;");
  //     mysql_query(conn, "DELETE FROM designproperty;");
  //     mysql_query(conn, "DELETE FROM component;");
  //     mysql_query(conn, "DELETE FROM componentproperty;");
  //     mysql_query(conn, "DELETE FROM property;");
  // end TEMP HACK
  if(conn != NULL){
    mysql_close(conn);
    conn = NULL;
  }
  unlock();
}

bool MysqlPersistence::saveGameInfo(){
  lock();
  mysql_query(conn, "DELETE FROM gameinfo;");
  unlock();
  try {
    std::ostringstream querybuilder;
    Game* game = Game::getGame();
    querybuilder << "INSERT INTO gameinfo VALUES ('" << addslashes(game->getKey()) << "', ";
    querybuilder << game->getGameStartTime() << ", " << game->getTurnNumber();
    querybuilder << ", '" << game->getTurnName() << "');";
    MysqlQuery query( conn, querybuilder.str(); );
  } catch ( MysqlException& e ) {
    return false;
  }
  return true;
}

bool MysqlPersistence::retrieveGameInfo(){
  try {
    MysqlQuery query( "SELECT * FROM gameinfo;" );
    Game* game = Game::getGame();
    game->setKey(query->get(0));
    game->setGameStartTime(query->getU64(1));
    game->setTurnNumber(query->getInt(2));
    game->setTurnName(query->get(3));
  } catch ( MysqlException& e ) { return false; }
  return true;
}

bool MysqlPersistence::saveObject(IGObject::Ptr ob){
  try {
    std::ostringstream querybuilder;
    uint32_t turn = Game::getGame()->getTurnNumber();
    uint32_t obid = ob->getID();

    querybuilder << "DELETE FROM object WHERE objectid = " << obid << " AND turnnum = " << turn << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "INSERT INTO object VALUES (" << obid << ", " << turn << ", ";
    querybuilder << (ob->isAlive() ? 1 : 0) << ", " << ob->getType() << ", ";
    querybuilder << "'" << addslashes(ob->getName()) << "', '" << addslashes(ob->getDescription()) << "', ";
    querybuilder << ob->getParent() << ", " << ob->getModTime() << ");";
    singleQuery(  querybuilder.str() );

    bool rtv = true;
    //store type-specific information
    if(ob->isAlive()){
      try{
        ObjectParameterGroup::Map groups = ob->getParameterGroups();
        for(ObjectParameterGroup::Map::iterator itcurr = groups.begin();
            itcurr != groups.end(); ++itcurr){
          ObjectParameterGroup::ParameterList params = itcurr->second->getParameters();
          uint32_t ppos = 0;
          for(ObjectParameterGroup::ParameterList::iterator paramcurr = params.begin();
              paramcurr != params.end(); ++paramcurr){
            ObjectParameter* parameter = *(paramcurr);
            switch(parameter->getType()){
              case obpT_Position_3D:
                updatePosition3dObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<Position3dObjectParam*>(parameter));
                break;
              case obpT_Velocity:
                updateVelocity3dObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<Velocity3dObjectParam*>(parameter));
                break;
              case obpT_Order_Queue:
                updateOrderQueueObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<OrderQueueObjectParam*>(parameter));
                break;
              case obpT_Resource_List:
                updateResourceListObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<ResourceListObjectParam*>(parameter));
                break;
              case obpT_Reference:
                updateReferenceObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<ReferenceObjectParam*>(parameter));
                break;
              case obpT_Reference_Quantity_List:
                updateRefQuantityListObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<RefQuantityListObjectParam*>(parameter));
                break;
              case obpT_Integer:
                updateIntegerObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<IntegerObjectParam*>(parameter));
                break;
              case obpT_Size:
                updateSizeObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<SizeObjectParam*>(parameter));
                break;
              default:
                Logger::getLogger()->error("Unknown ObjectParameter type %d", parameter->getType());
                throw std::exception();
                break;
            }
            ppos++;
          }
        }


        //TODO: useless ;)
      }catch(std::exception& e){
        rtv = false;
      }
    }
    ob->setIsDirty(!rtv);

    return rtv;
  } catch ( MysqlException& e ) { 
    ob->setIsDirty(true);
    return false; 
  }
  return true;
}

IGObject::Ptr MysqlPersistence::retrieveObject(uint32_t obid){
  try {
    std::ostringstream querybuilder;
    uint32_t turn = Game::getGame()->getTurnNumber();

    querybuilder << "SELECT * FROM object WHERE objectid = " << obid << " AND turnnum <= " << turn;
    querybuilder << " ORDER BY turnnum DESC LIMIT 1;";
    MysqlQuery query( querybuilder );
    query->nextRow();

    //children object ids
    querybuilder.str("");
    querybuilder << "SELECT object.objectid FROM object JOIN (SELECT objectid, MAX(turnnum) AS maxturnnum FROM object WHERE turnnum <= " << turn << " GROUP BY objectid) AS maxx ON (object.objectid=maxx.objectid AND object.turnnum = maxx.maxturnnum) WHERE parentid = " << obid << " ;";
    MysqlQuery cquery( querybuilder );

    IGObject::Ptr object( new IGObject(obid) );

    Game::getGame()->getObjectTypeManager()->setupObject(object, query->getInt(3));
    object->setIsAlive(query->getInt(2) == 1);
    object->setName(query->get(4));
    object->setDescription(query->get(5));
    object->setParent(query->getInt(6));

    while(cquery->nextRow()){
      uint32_t childid = cquery->getInt(0);
      DEBUG("childid: %d", childid);
      if(childid != object->getID())
        object->addContainedObject(childid);
    }
    DEBUG("num children: %d", object->getContainedObjects().size());

    // fetch type-specific information
    if(object->isAlive()){
      try{
        std::map<uint32_t, ObjectParameterGroupPtr> groups = object->getParameterGroups();
        for(std::map<uint32_t, ObjectParameterGroupPtr>::iterator itcurr = groups.begin();
            itcurr != groups.end(); ++itcurr){
          ObjectParameterGroupData::ParameterList params = itcurr->second->getParameters();
          uint32_t ppos = 0;
          for(ObjectParameterGroupData::ParameterList::iterator paramcurr = params.begin();
              paramcurr != params.end(); ++paramcurr){
            ObjectParameter* parameter = *(paramcurr);
            switch(parameter->getType()){
              case obpT_Position_3D:
                retrievePosition3dObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<Position3dObjectParam*>(parameter));
                break;
              case obpT_Velocity:
                retrieveVelocity3dObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<Velocity3dObjectParam*>(parameter));
                break;
              case obpT_Order_Queue:
                retrieveOrderQueueObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<OrderQueueObjectParam*>(parameter));
                break;
              case obpT_Resource_List:
                retrieveResourceListObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<ResourceListObjectParam*>(parameter));
                break;
              case obpT_Reference:
                retrieveReferenceObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<ReferenceObjectParam*>(parameter));
                break;
              case obpT_Reference_Quantity_List:
                retrieveRefQuantityListObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<RefQuantityListObjectParam*>(parameter));
                break;
              case obpT_Integer:
                retrieveIntegerObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<IntegerObjectParam*>(parameter));
                break;
              case obpT_Size:
                retrieveSizeObjectParam(obid, turn, 0, itcurr->first, ppos, static_cast<SizeObjectParam*>(parameter));
                break;
              default:
                Logger::getLogger()->error("Unknown ObjectParameter type %d", parameter->getType());
                throw std::exception();
                break;
            }
            ppos++;
          }
        }

      }catch(std::exception* e){
        delete object;
        return NULL;
      }
    }

    object->setModTime( query->getU64(7));
    object->setIsDirty(false);

  } catch ( MysqlException& e ) { 
    return IGObject::Ptr(); 
  }
  return object;
}

uint32_t MysqlPersistence::getMaxObjectId(){
  try {
    return valueQuery( "SELECT MAX(objectid) FROM object;");
  } catch ( MysqlException& ) return 0;
}

IsSet MysqlPersistence::getObjectIds(){
  try {
    MysqlQuery query(conn, "SELECT objectid FROM object;");
    IdSet vis;
    while ( query->nextRow() ) {
      vis.insert( query->getInt(0) );
    }
    return vis;
  } catch ( MysqlException& ) return IdSet();
}

bool MysqlPersistence::saveOrderQueue(const OrderQueue::Ptr oq){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO orderqueue VALUES (" << oq->getQueueId() << ", " << oq->getObjectId() << ", ";
    querybuilder << (oq->isActive() ? 1 : 0) << ", " << (oq->isRepeating() ? 1 : 0) << ", " << oq->getModTime() << ");";
    singleQuery( querybuilder.str() );

    insertList( "orderslot", oq->getQueueId(), oq->getOrderSlots() );
    insertSet ( "orderqueueowner", oq->getQueueId(), oq->getOwner() );
    insertSet ( "orderqueueallowedordertype", oq->getQueueId(), oq->getAllowedOrderTypes() );
    return true;
  } catch ( MysqlException& ) return false;
}

bool MysqlPersistence::updateOrderQueue(const OrderQueue::Ptr oq){
  try {
    std::ostringstream querybuilder;
    querybuilder << "UPDATE orderqueue set objectid=" << oq->getObjectId() << ", active=" << (oq->isActive() ? 1 : 0);
    querybuilder << ", repeating=" << (oq->isRepeating() ? 1 : 0) << ", modtime=" << oq->getModTime()<< " WHERE queueid=" << oq->getQueueId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM orderslot WHERE queueid=" << oq->getQueueId() <<";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM orderqueueowner WHERE queueid=" << oq->getQueueId() <<";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM orderqueueallowedtype WHERE queueid=" << oq->getQueueId() <<";";
    singleQuery( querybuilder.str() );

    insertList( "orderslot", oq->getQueueId(), oq->getOrderSlots() );
    insertSet ( "orderqueueowner", oq->getQueueId(), oq->getOwner() );
    insertSet ( "orderqueueallowedordertype", oq->getQueueId(), oq->getAllowedOrderTypes() );

    return true;
  } catch ( MysqlException& ) return false;
}

OrderQueue::Ptr MysqlPersistence::retrieveOrderQueue(uint32_t oqid){
  try {
    std::ostringstream querybuilder;

    {
      querybuilder << "SELECT * FROM orderqueue WHERE queueid=" << oqid << ";";

      MysqlQuery query( querybuilder.str() );
      OrderQueue::Ptr oq( new OrderQueue(oqid, query->getInt(1), 0) );
      oq->setActive(query->getInt(2) == 1);
      oq->setRepeating(query->getInt(3) == 1);
      oq->setModTime(query->getU64(4));
    }

    querybuilder.str("");
    querybuilder << "SELECT orderid FROM orderslot WHERE queueid=" << oqid <<" ORDER BY slot;";a

    IdList oolist = idListQuery( querybuilder.str() );
    uint32_t max = std::max_element( oolist.begin(), oolist.end() );
    oq->setOrderSlots(oolist);
    oq->setNextOrderId(max+1);

    querybuilder.str("");
    querybuilder << "SELECT playerid FROM orderqueueowner WHERE queueid=" << oqid <<";";
    oq->setOwners( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT ordertype FROM orderqueueallowedtype WHERE queueid=" << oqid <<";";
    oq->setAllowedOrderTypes( idSetQuery( querybuilder.str() ) );

    return oq;
  } catch ( MysqlException& ) {
    return OrderQueue::Ptr();
  }
}

bool MysqlPersistence::removeOrderQueue(uint32_t oqid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM orderslot WHERE queueid=" << oqid <<";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM orderqueueowner WHERE queueid=" << oqid <<";";
    singleQuery( querybuilder.str() );

    return true;
  } catch( MysqlException& ) { return false; }
}

IdSet MysqlPersistence::getOrderQueueIds(){
  try {
    return idSetQuery( "SELECT queueid FROM orderqueue;");
  } catch( MysqlException& ) { return IdSet(); }
}

uint32_t MysqlPersistence::getMaxOrderQueueId(){
  try {
    return valueQuery( "SELECT MAX(queueid) FROM orderqueue;" );
  } catch( MysqlException& ) { return 0; }
}

bool MysqlPersistence::saveOrder(uint32_t queueid, uint32_t ordid, Order* ord){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO ordertype VALUES (" << queueid << ", " << ordid << ", " << ord->getType() << ", " << ord->getTurns() << ");";
    singleQuery( querybuilder.str() );

    insertMap( "orderresource", queueid, ordid, ord->getResources() );

    //store parameters
    uint32_t parampos = 0;
    std::list<OrderParameter*> params = ord->getParameters();
    for(std::list<OrderParameter*>::iterator itcurr = params.begin(); itcurr != params.end(); ++itcurr){
      switch((*itcurr)->getType()){
        case opT_Space_Coord_Abs:
          updateSpaceCoordParam(queueid, ordid, parampos, static_cast<SpaceCoordParam*>(*itcurr));
          break;
        case opT_Time:
          updateTimeParameter(queueid, ordid, parampos, static_cast<TimeParameter*>(*itcurr));
          break;
        case opT_Object_ID:
          updateObjectOrderParameter(queueid, ordid, parampos, static_cast<ObjectOrderParameter*>(*itcurr));
          break;
        case opT_List:
          updateListParameter(queueid, ordid, parampos, static_cast<ListParameter*>(*itcurr));
          break;
        case opT_String:
          updateStringParameter(queueid, ordid, parampos, static_cast<StringParameter*>(*itcurr));
          break;
        default:
          Logger::getLogger()->error("MysqlPersistence: unknown order parameter type at save");
          return false;
      }
      parampos++;
    }

    return true;

  } catch( MysqlException& ) { return false; }
}

bool MysqlPersistence::updateOrder(uint32_t queueid, uint32_t ordid, Order* ord){
  try {
    std::ostringstream querybuilder;
    querybuilder << "UPDATE ordertype SET type = " << ord->getType() << ", turns=" << ord->getTurns() << " WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    singleQuery( querybuilder.str() );

    insertMap( "orderresource", queueid, ordid, ord->getResources() );

    //update parameters
    uint32_t parampos = 0;
    std::list<OrderParameter*> params = ord->getParameters();
    for(std::list<OrderParameter*>::iterator itcurr = params.begin(); itcurr != params.end(); ++itcurr){
      switch((*itcurr)->getType()){
        case opT_Space_Coord_Abs:
          updateSpaceCoordParam(queueid, ordid, parampos, static_cast<SpaceCoordParam*>(*itcurr));
          break;
        case opT_Time:
          updateTimeParameter(queueid, ordid, parampos, static_cast<TimeParameter*>(*itcurr));
          break;
        case opT_Object_ID:
          updateObjectOrderParameter(queueid, ordid, parampos, static_cast<ObjectOrderParameter*>(*itcurr));
          break;
        case opT_List:
          updateListParameter(queueid, ordid, parampos, static_cast<ListParameter*>(*itcurr));
          break;
        case opT_String:
          updateStringParameter(queueid, ordid, parampos, static_cast<StringParameter*>(*itcurr));
          break;
        default:
          Logger::getLogger()->error("MysqlPersistence: unknown order parameter type at update");
          return false;
      }
      parampos++;
    }
    return true;
  } catch( MysqlException& ) { return false; }
}

Order* MysqlPersistence::retrieveOrder(uint32_t queueid, uint32_t ordid){
  Order* order = NULL;
  try {
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT type,turns FROM ordertype WHERE queueid = " << queueid << " AND orderid = " << ordid << ";";
      MysqlQuery query( conn, querybuilder.str() );

      order = Game::getGame()->getOrderManager()->createOrder( query->getInt(0) );
      order->setTurns( query->getInt(1) );
      order->setOrderQueueId(queueid);
    }

    //fetch resources
    {
      querybuilder.str("");
      querybuilder << "SELECT resourceid, amount FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      while( query->nextRow() ){
        order->addResource(query->getInt(0), query->getInt(1));
      }
    }
    //fetch parameters
    uint32_t parampos = 0;
    std::list<OrderParameter*> params = order->getParameters();
    for(std::list<OrderParameter*>::iterator itcurr = params.begin(); itcurr != params.end(); ++itcurr){
      switch((*itcurr)->getType()){
        case opT_Space_Coord_Abs:
          retrieveSpaceCoordParam(queueid, ordid, parampos, static_cast<SpaceCoordParam*>(*itcurr));
          break;
        case opT_Time:
          retrieveTimeParameter(queueid, ordid, parampos, static_cast<TimeParameter*>(*itcurr));
          break;
        case opT_Object_ID:
          retrieveObjectOrderParameter(queueid, ordid, parampos, static_cast<ObjectOrderParameter*>(*itcurr));
          break;
        case opT_List:
          retrieveListParameter(queueid, ordid, parampos, static_cast<ListParameter*>(*itcurr));
          break;
        case opT_String:
          retrieveStringParameter(queueid, ordid, parampos, static_cast<StringParameter*>(*itcurr));
          break;
        default:
          Logger::getLogger()->error("MysqlPersistence: unknown order parameter type at retrieve");
          return false;
      }
      parampos++;
    }

    return order;
  } catch( MysqlException& ) { 
    delete order;
    return false; 
  }
}

bool MysqlPersistence::removeOrder(uint32_t queueid, uint32_t ordid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM ordertype WHERE queueid=" << queueid << " AND orderid=" << ordid <<";";
    uint32_t ordertype = valueQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM ordertype WHERE queueid= " << queueid << " AND orderid=" << ordid << ";";
    singleQuery( querybuilder.str() );

    //remove resources
    querybuilder.str("");
    querybuilder << "DELETE FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    singleQuery( querybuilder.str() );

    //remove parameters
    uint32_t parampos = 0;
    Order* order = Game::getGame()->getOrderManager()->createOrder(ordertype);
    std::list<OrderParameter*> params = order->getParameters();
    for(std::list<OrderParameter*>::iterator itcurr = params.begin(); itcurr != params.end(); ++itcurr){
      switch((*itcurr)->getType()){
        case opT_Space_Coord_Abs:
          removeSpaceCoordParam(queueid, ordid, parampos);
          break;
        case opT_Time:
          removeTimeParameter(queueid, ordid, parampos);
          break;
        case opT_Object_ID:
          removeObjectOrderParameter(queueid, ordid, parampos);
          break;
        case opT_List:
          removeListParameter(queueid, ordid, parampos);
          break;
        case opT_String:
          removeStringParameter(queueid, ordid, parampos);
          break;
        default:
          Logger::getLogger()->error("MysqlPersistence: unknown order parameter type at remove");
          return false;
      }
      parampos++;
    }
    delete order;
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

bool MysqlPersistence::updateSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamspace WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );

  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamspace VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << scp->getPosition().getX() << ", " << scp->getPosition().getY() << ", ";
  querybuilder << scp->getPosition().getZ() <<");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT posx,posy,posz FROM orderparamspace WHERE queueid = " << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  MysqlQuery query( conn, querybuilder.str() );
  scp->setPosition(Vector3d(atoll(query->get(0)), atoll(query->get(1)), atoll(query->get(2))));
  return true;
}

bool MysqlPersistence::removeSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamspace WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::updateListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );

  IdMap list = lp->getList();
  if(!list.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderparamlist VALUES ";
    for(IdMap::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
      if(itcurr != list.begin())
        querybuilder << ", ";
      querybuilder << "(" << queueid << ", " << ordid << ", " << pos << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
    }
    querybuilder << ";";
    singleQuery( querybuilder.str() );
  }
  return true;
}

bool MysqlPersistence::retrieveListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT listid, amount FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position = " << pos << ";";
  lp->setList( idMapQuery( querybuilder.str() ) );
  return true;
}

bool MysqlPersistence::removeListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::updateObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamobject WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );

  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamobject VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << ob->getObjectId() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob) {
  std::ostringstream querybuilder;
  querybuilder << "SELECT objectid FROM orderparamobject WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  ob->setObjectId(valueQuery( querybuilder.str() ));
  return true;
}

bool MysqlPersistence::removeObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamobject WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::updateStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamstring WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );

  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamstring VALUES (" << queueid << ", " << ordid << ", " << pos << ", '";
  querybuilder << addslashes(st->getString()) <<"');";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st){
  std::ostringstream querybuilder;
  querybuilder << "SELECT thestring FROM orderparamstring WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  MysqlQuery query( conn, querybuilder.str() );
  st->setString( query->get(0));
  return true;
}

bool MysqlPersistence::removeStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamstring WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::updateTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamtime WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );

  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamtime VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << tp->getTime() <<");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT turns FROM orderparamtime WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  tp->setTime( valueQuery( querybuilder.str() ) );
  return true;
}

bool MysqlPersistence::removeTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamtime WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::saveBoard(Board::Ptr board){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO board VALUES (" << board->getId() << ", '" << addslashes(board->getName()) << "', '";
    querybuilder << addslashes(board->getDescription()) << "', " << board->getNumMessages() << ", ";
    querybuilder << board->getModTime() <<");";
    singleQuery( querybuilder.str() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

bool MysqlPersistence::updateBoard(Board::Ptr board){
  try {
    std::ostringstream querybuilder;
    querybuilder << "UPDATE board SET name='" << addslashes(board->getName()) << "', description='" << addslashes(board->getDescription());
    querybuilder << "', nummessages=" << board->getNumMessages() << ", modtime=" << board->getModTime();
    querybuilder << " WHERE boardid=" << board->getId() << ";";
    singleQuery( querybuilder.str() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Board::Ptr MysqlPersistence::retrieveBoard(uint32_t boardid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM board WHERE boardid = " << boardid << ";";
    MysqlQuery query( conn, querybuilder.str() );
    Board::Ptr board( new Board( boardid, query->get(1), query->get(2) ) );
    board->setPersistenceData(query->getInt(3),query->getU64(4));
    return board;
  } catch( MysqlException& ) { 
    return Board::Ptr(); 
  }
}

uint32_t MysqlPersistence::getMaxBoardId(){
  try {
    return valueQuery( "SELECT MAX(boardid) FROM board;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getBoardIds(){
  try {
    return idSetQuery( "SELECT boardid FROM board;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveMessage( boost::shared_ptr< Message > msg){
  try {
    uint32_t msgid = msg->getId();
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO message VALUES (" << msgid << ", '" << addslashes(msg->getSubject()) << "', '";
    querybuilder << addslashes(msg->getBody()) << "', " << msg->getTurn() << ");";
    singleQuery( querybuilder.str() );

    Message::References refs = msg->getReferences();
    if(!refs.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO messagereference VALUES ";
      for(Message::References::iterator itcurr = refs.begin(); itcurr != refs.end(); ++itcurr){
        if(itcurr != refs.begin())
          querybuilder << ", ";
        querybuilder << "(" << msgid << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
      }
      singleQuery( querybuilder.str() );
    }
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

boost::shared_ptr< Message > MysqlPersistence::retrieveMessage(uint32_t msgid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM message WHERE messageid = " << msgid << ";";
    Message::Ptr msg;
    {
      MysqlQuery query( conn, querybuilder.str() );
      msg.reset( new Message() );
      msg->setSubject( query->get(1) );
      msg->setBody( query->get(2) );
      msg->setTurn( query->get(3) );
    }

    querybuilder.str("");
    querybuilder << "SELECT type,refid FROM messagereference WHERE messageid = " << msgid << ";";
    {
      MysqlQuery query( conn, querybuilder.str() );
      while(query->nextRow()){
        msg->addReference((RefSysType)query->getInt(0),query->getInt(1));
      }
    }
    return msg;
  } catch( MysqlException& ) { 
    return Message::Ptr(); 
  }
}

bool MysqlPersistence::removeMessage(uint32_t msgid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM message WHERE messageid=" << msgid << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM messagereference WHERE messageid=" << msgid << ";";
    singleQuery( querybuilder.str() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

bool MysqlPersistence::saveMessageList(uint32_t bid, IdList list){
  try {
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM messageslot WHERE boardid=" << bid <<";";
    singleQuery( querybuilder.str() );
    insertList( "messageslot", bid, list );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

IdList MysqlPersistence::retrieveMessageList(uint32_t bid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT messageid FROM messageslot WHERE boardid=" << bid <<" ORDER BY slot;";
    return idListQuery( querybuilder.str() );
  } catch( MysqlException& ) { 
    return IdList(); 
  }
}

uint32_t MysqlPersistence::getMaxMessageId(){
  try {
    return valueQuery( "SELECT MAX(messageid) FROM message;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

bool MysqlPersistence::saveResource(ResourceDescription::Ptr res){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO resourcedesc VALUES (" << res->getResourceType() << ", '" << addslashes(res->getNameSingular()) << "', '";
    querybuilder << addslashes(res->getNamePlural()) << "', '" << addslashes(res->getUnitSingular()) << "', '";
    querybuilder << addslashes(res->getUnitPlural()) << "', '" << addslashes(res->getDescription()) << "', ";
    querybuilder << res->getMass() << ", " << res->getVolume() << ", " << res->getModTime() << ");";
    singleQuery( querybuilder.str() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

ResourceDescription::Ptr MysqlPersistence::retrieveResource(uint32_t restype){
  ResourceDescription::Ptr res;
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM resourcedesc WHERE resourcetype = " << restype << ";";
    MysqlQuery query( conn, querybuilder.str() );

    res.reset( new ResourceDescription() );
    res->setResourceType(restype);
    res->setNameSingular(query->get(1));
    res->setNamePlural(query->get(2));
    res->setUnitSingular(query->get(3));
    res->setUnitPlural(query->get(4));
    res->setDescription(query->get(5));
    res->setMass(query->getInt(6));
    res->setVolume(query->getInt(7));
    res->setModTime(query->getU64(8));
    return res;
  } catch( MysqlException& ) { 
    return ResourceDescription::Ptr(); 
  }
}

uint32_t MysqlPersistence::getMaxResourceId(){
  try {
    return valueQuery( "SELECT MAX(resourcetype) FROM resourcedesc;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getResourceIds(){
  try {
    return idSetQuery( "SELECT resourcetype FROM resourcedesc;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::savePlayer(Player::Ptr player){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO player VALUES (" << player->getID() << ", '" << addslashes(player->getName()) << "', '";
    querybuilder << addslashes(player->getPass()) << "', '" << addslashes(player->getEmail()) << "', '";
    querybuilder << addslashes(player->getComment()) << "', " << player->getBoardId() << ", ";
    querybuilder << ((player->isAlive()) ? 1 : 0) << ", " << player->getModTime() << ");";
    singleQuery( querybuilder.str() );

    PlayerView::Ptr playerview = player->getPlayerView();
    insertMap( "playerscore", player->getID(), player->getAllScores() );
    insertSet( "playerdesignusable", player->getID(), playerview->getUsableDesigns() );   
    insertSet( "playercomponentusable", player->getID(), playerview->getUsableComponents() );   
    insertSet( "playerobjectowned", player->getID(), playerview->getOwnedObjects() );   
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

bool MysqlPersistence::updatePlayer(Player::Ptr player){
  try {
    std::ostringstream querybuilder;
    querybuilder << "UPDATE player SET name='" << addslashes(player->getName()) << "', password='" << addslashes(player->getPass());
    querybuilder << "', email='" << addslashes(player->getEmail()) << "', comment='" << addslashes(player->getComment());
    querybuilder << "', boardid=" << player->getBoardId() << ", alive=" << ((player->isAlive()) ? 1 : 0);
    querybuilder << ", modtime=" << player->getModTime() << " WHERE playerid=" << player->getID() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playerscore WHERE playerid=" << player->getID() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignusable WHERE playerid=" << player->getID() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentusable WHERE playerid=" << player->getID() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playerobjectowned WHERE playerid=" << player->getID() << ";";
    singleQuery( querybuilder.str() );

    PlayerView::Ptr playerview = player->getPlayerView();
    insertMap( "playerscore", player->getID(), player->getAllScores() );
    insertSet( "playerdesignusable", player->getID(), playerview->getUsableDesigns() );   
    insertSet( "playercomponentusable", player->getID(), playerview->getUsableComponents() );   
    insertSet( "playerobjectowned", player->getID(), playerview->getOwnedObjects() );   
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Player::Ptr MysqlPersistence::retrievePlayer(uint32_t playerid){
  Player::Ptr player;
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM player WHERE playerid = " << playerid << ";";
    {
      MysqlQuery query( conn, querybuilder.str() );

      player.reset( new Player( playerid, query->get(1), query->get(2)) );
      player->setEmail(query->get(3));
      player->setComment(query->get(4));
      player->setBoardId(query->getInt(5));
      player->setIsAlive(query->getInt(6) == 1);
      player->setModTime = query->getU64(7);
    }
    {
      querybuilder.str("");
      querybuilder << "SELECT * FROM playerscore WHERE playerid = " << playerid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      while((row = mysql_fetch_row(res)) != NULL){
        player->setScore( query->getInt(1), query->getInt(2) );
      }
    }
    PlayerView::Ptr playerview = player->getPlayerView();
    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignview WHERE playerid = " << playerid << ";";
    playerview->setVisibleDesigns( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignusable WHERE playerid = " << playerid << ";";
    playerview->setUsableDesigns( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentview WHERE playerid = " << playerid << ";";
    playerview->setVisibleComponents( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentusable WHERE playerid = " << playerid << ";";
    playerview->setUsableComponents( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT DISTINCT objectid FROM playerobjectview WHERE playerid = " << playerid << ";";
    playerview->setVisibleObjects( idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT objectid FROM playerobjectowned WHERE playerid = " << playerid << ";";
    playerview->setOwnedObjects( idSetQuery( querybuilder.str() ) );

    return player;
  } catch( MysqlException& ) { 
    return NULL; 
  }
}

uint32_t MysqlPersistence::getMaxPlayerId(){
  try {
    return valueQuery( "SELECT MAX(playerid) FROM player;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getPlayerIds(){
  try {
    return idSetQuery( "SELECT playerid FROM player;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveCategory(Category::Ptr cat){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO category VALUES (" << cat->getCategoryId() << ", '" << cat->getName() << "', '";
    querybuilder << cat->getDescription() << "', " << cat->getModTime() << ");";
    singleQuery( querybuilder.str() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Category::Ptr MysqlPersistence::retrieveCategory(uint32_t catid){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM category WHERE categoryid = " << catid << ";";
    MysqlQuery query( conn, querybuilder.str() );
    Category::Ptr cat( new Category() );
    cat->setCategoryId(catid);
    cat->setName(query->get(1));
    cat->setDescription(query->get(2));
    cat->setModTime(query->getU64(3));
    return cat;
  } catch( MysqlException& ) { 
    return Category::Ptr();
  }
}

uint32_t MysqlPersistence::getMaxCategoryId(){
  try {
    return valueQuery( "SELECT MAX(categoryid) FROM category;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getCategoryIds(){
  try {
    return idSetQuery( "SELECT categoryid FROM category;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveDesign(Design::Ptr design){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO design VALUES (" << design->getDesignId() << ", " << design->getCategoryId() << ", '";
    querybuilder << addslashes(design->getName()) << "', '" << addslashes(design->getDescription()) << "', " << design->getOwner() << ", ";
    querybuilder << design->getInUse() << ", " << design->getNumExist() << ", " << design->isValid() << ", '";
    querybuilder << addslashes(design->getFeedback()) << "', " << design->getModTime() << ");";
    singleQuery( querybuilder.str() );

    insertMap( "designcomponent", design->getDesignId(), design->getComponents() );

    PropertyValue::Map proplist = design->getPropertyValues();
    if(!proplist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO designproperty VALUES ";
      for( PropertyValue::Map::iterator itcurr = proplist.begin(); itcurr != proplist.end(); ++itcurr){
        if(itcurr != proplist.begin())
          querybuilder << ", ";
        PropertyValue pv = itcurr->second;
        querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << pv.getValue() << ", '";
        querybuilder << addslashes(pv.getDisplayString()) << "')";
      }
      querybuilder << ";";
      singleQuery( querybuilder.str() );
    }
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

bool MysqlPersistence::updateDesign(Design::Ptr design){
  try {
    std::ostringstream querybuilder;
    querybuilder << "UPDATE design SET categoryid=" << design->getCategoryId() << ", name='";
    querybuilder << addslashes(design->getName()) << "', description='" << addslashes(design->getDescription()) << "', owner=";
    querybuilder << design->getOwner() << ", inuse=" << design->getInUse() << ", numexist=" << design->getNumExist() << ", valid=";
    querybuilder << design->isValid() << ", feedback='" << addslashes(design->getFeedback());
    querybuilder << "', modtime=" << design->getModTime() << " WHERE designid=" << design->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM designcomponent WHERE designid=" << design->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM designproperty WHERE designid=" << design->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    insertMap( "designcomponent", design->getDesignId(), design->getComponents() );

    PropertyValue::Map proplist = design->getPropertyValues();
    if(!proplist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO designproperty VALUES ";
      for(std::map<uint32_t, PropertyValue::Map::iterator itcurr = proplist.begin(); itcurr != proplist.end(); ++itcurr){
        if(itcurr != proplist.begin())
          querybuilder << ", ";
        PropertyValue pv = itcurr->second;
        querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << pv.getValue() << ", '";
        querybuilder << addslashes(pv.getDisplayString()) << "')";
      }
      querybuilder << ";";
      singleQuery( querybuilder.str() );
    }
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Design::Ptr MysqlPersistence::retrieveDesign(uint32_t designid){
  Design::Ptr design;
  try {
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT * FROM design WHERE designid = " << designid << ";";
      MysqlQuery query( conn, querybuilder.str() );

      design.reset( new Design() );
      design->setDesignId(designid);
      design->setCategoryId(query->getInt(1));
      design->setName(query->get(2));
      design->setDescription(query->get(3));
      design->setOwner(query->getInt(4));
      design->setInUse(query->getInt(5));
      design->setNumExist(query->getInt(6));
      design->setValid(query->getInt(7),query->getInt(8));
      design->setModTime(query->getU64(9));
    }

    querybuilder.str("");
    querybuilder << "SELECT componentid,count FROM designcomponent WHERE designid = " << designid << ";";a
      design->setComponents(idMapQuery(querybuilder.str()));

    {
      querybuilder.str("");
      querybuilder << "SELECT propertyid,value,displaystring FROM designproperty WHERE designid = " << designid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      PropertyValue::Map pvlist;
      while(query->nextRow()){
        PropertyValue pv( atoi(query->get(0)), atof(query->get(1)));
        pv.setDisplayString(query->get(2));
        pvlist[pv.getPropertyId()] = pv;
      }
      design->setPropertyValues(pvlist);
    }

    return design;
  } catch( MysqlException& ) {
    return Design::Ptr(); 
  }
}

uint32_t MysqlPersistence::getMaxDesignId(){
  try {
    return valueQuery( "SELECT MAX(designid) FROM design;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getDesignIds(){
  try {
    return idSetQuery( "SELECT designid FROM design;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveComponent(Component::Ptr comp){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO component VALUES (" << comp->getComponentId();
    querybuilder<< ", '" << addslashes(comp->getName()) << "', '" << addslashes(comp->getDescription()) << "', '";
    querybuilder << addslashes(comp->getTpclRequirementsFunction()) << "', " << comp->getModTime() << ");";
    singleQuery( querybuilder.str() );

    insertSet( "componentcat", comp->getId(), comp->getCategoryIds() );

    std::map<uint32_t, std::string> proplist = comp->getPropertyList();
    if(!proplist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO componentproperty VALUES ";
      for(std::map<uint32_t, std::string>::iterator itcurr = proplist.begin(); itcurr != proplist.end(); ++itcurr){
        if(itcurr != proplist.begin())
          querybuilder << ", ";
        querybuilder << "(" << comp->getComponentId() << ", " << itcurr->first << ", '" << addslashes(itcurr->second) << "')";
      }
      querybuilder << ";";
      singleQuery( querybuilder.str() );
    }
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Component::Ptr MysqlPersistence::retrieveComponent(uint32_t compid){
  Component comp;
  try {
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT * FROM component WHERE componentid = " << compid << ";";
      MysqlQuery query( conn, querybuilder.str() );

      comp.reset( new Component() );
      comp->setComponentId(compid);
      comp->setName(query->get(1));
      comp->setDescription(query->get(2));
      comp->setTpclRequirementsFunction(query->get(3));
      comp->setModTime(query->getU64(4));
    }

    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM componentcat WHERE componentid = " << compid << ";";
    comp->setCategoryIds(idSetQuery( querybuilder.str() ) );

    {
      querybuilder.str("");
      querybuilder << "SELECT propertyid,tpclvaluefunc FROM componentproperty WHERE componentid = " << compid << ";";
      MysqlQuery query( conn, querybuilder.str() );

      std::map<uint32_t, std::string> pvlist;
      while(query->nextRow()) {
        pvlist[query->getInt(0)] = query->get(1);
      }
      comp->setPropertyList(pvlist);
    }
    return comp;
  } catch( MysqlException& ) {
    return Component::Ptr(); 
  }
}

uint32_t MysqlPersistence::getMaxComponentId(){
  try {
    return valueQuery( "SELECT MAX(componentid) FROM component;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}


IdSet MysqlPersistence::getComponentIds(){
  try {
    return idSetQuery( "SELECT componentid FROM component;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveProperty(Property::Ptr prop){
  try {
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO property VALUES (" << prop->getPropertyId() << ", ";
    querybuilder << prop->getRank() << ", '" << addslashes(prop->getName()) << "', '" << addslashes(prop->getDisplayName());
    querybuilder << "', '" << addslashes(prop->getDescription()) << "', '" << addslashes(prop->getTpclDisplayFunction()) << "', '";
    querybuilder << addslashes(prop->getTpclRequirementsFunction()) << "', " << prop->getModTime() << ");";
    singleQuery( querybuilder.str() );
    insertSet( "propertycat", prop->getPropertyId(), prop->getCategoryIds() );
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

Property::Ptr MysqlPersistence::retrieveProperty(uint32_t propid){
  Property::Ptr prop;
  try {
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT * FROM property WHERE propertyid = " << propid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      prop.reset( new Property() );
      prop->setPropertyId(propid);
      prop->setRank(query->getInt(1));
      prop->setName(query->get(2));
      prop->setDisplayName(query->get(3));
      prop->setDescription(query->get(4));
      prop->setTpclDisplayFunction(query->get(5));
      prop->setTpclRequirementsFunction(query->get(6));
      prop->setModTime(query->getU64(7));
    }

    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM propertycat WHERE propertyid = " << propid << ";";
    prop->setCategoryIds(idSetQuery( querybuilder.str() ) );
    return prop;
  } catch( MysqlException& ) {
    return Property::Ptr(); 
  }
}

uint32_t MysqlPersistence::getMaxPropertyId(){
  try {
    return valueQuery( "SELECT MAX(propertyid) FROM property;" );
  } catch( MysqlException& ) { 
    return 0; 
  }
}

IdSet MysqlPersistence::getPropertyIds(){
  try {
    return idSetQuery( "SELECT propertyid FROM property;" );
  } catch( MysqlException& ) { 
    return IdSet(); 
  }
}

bool MysqlPersistence::saveObjectView(uint32_t playerid, ObjectView::Ptr ov){
  try {
    std::ostringstream querybuilder;
    uint32_t turnnum =  Game::getGame()->getTurnNumber();
    querybuilder << "DELETE FROM playerobjectview WHERE playerid=" << playerid << " AND objectid=" << ov->getObjectId() << " AND turnnum = " << turnnum << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "INSERT INTO playerobjectview VALUES (" << playerid << ", " << ov->getObjectId() << ", ";
    querybuilder << turnnum << ", " << ((ov->isCompletelyVisible()) ? 1 : 0) << ", "; 
    querybuilder << ((ov->isGone()) ? 1 : 0) << ", " << ((ov->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(ov->getVisibleName()) << "', " << ((ov->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(ov->getVisibleDescription()) << "', " << ov->getModTime() << ");";
    singleQuery( querybuilder.str() );

    //params?

    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

ObjectView::Ptr MysqlPersistence::retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn){
  try {
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM playerobjectview WHERE playerid = " << playerid << " AND objectid = " << objectid << " AND turnnum <= " << turn << " ORDER BY turnnum DESC LIMIT 1;";
    MysqlQuery query( conn, querybuilder.str() );
    ObjectView::Ptr obj( new ObjectView() );
    obj->setObjectId(objectid);
    obj->setCompletelyVisible(query->getInt(3) == 1);
    obj->setGone(query->getInt(4) == 1);
    obj->setCanSeeName(query->getInt(5) == 1);
    obj->setVisibleName(query->get(6));
    obj->setCanSeeDescription(query->getInt(7) == 1);
    obj->setVisibleDescription(query->get(8));
    obj->setModTime(query->getU64(9));
    return obj;
  } catch( MysqlException& ) {
    return ObjectView::Ptr(); 
  }
}

bool MysqlPersistence::saveDesignView(uint32_t playerid, DesignView::Ptr dv){
  try {
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM playerdesignview WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignviewcomp WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignviewprop WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "INSERT INTO playerdesignview VALUES (" << playerid << ", " << dv->getDesignId() << ", ";
    querybuilder << ((dv->isCompletelyVisible()) ? 1 : 0) << ", " << ((dv->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(dv->getVisibleName()) << "', " << ((dv->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(dv->getVisibleDescription()) << "', " << ((dv->canSeeNumExist()) ? 1 : 0) << ", ";
    querybuilder << dv->getVisibleNumExist() << ", " << ((dv->canSeeOwner()) ? 1 : 0) << ", ";
    querybuilder << dv->getVisibleOwner() << ", " << dv->getModTime() << ");";
    singleQuery( querybuilder.str() );

    insertMap( "playerdesignviewcomp", playerid, dv->getDesignId(), dv->getVisibleComponents() );

    PropertyValue::Map proplist = dv->getVisiblePropertyValues();
    if(!proplist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO playerdesignviewprop VALUES ";
      for(std::map<uint32_t, PropertyValue>::iterator itcurr = proplist.begin(); itcurr != proplist.end();
          ++itcurr){
        if(itcurr != proplist.begin())
          querybuilder << ", ";
        querybuilder << "(" << playerid << ", " << dv->getDesignId() << ", " << (itcurr->second.getPropertyId());
        querybuilder << ", '" << addslashes(itcurr->second.getDisplayString()) << "')";
      }
      querybuilder << ";";
      singleQuery( querybuilder.str() );
    }
    return true;
  } catch( MysqlException& ) { 
    return false; 
  }
}

DesignView::Ptr MysqlPersistence::retrieveDesignView(uint32_t playerid, uint32_t designid){
  try {
    DesignView::Ptr design( new DesignView() );
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT * FROM playerdesignview WHERE playerid = " << playerid << " AND designid = " << designid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      design->setDesignId(designid);
      design->setIsCompletelyVisible(query->getInt(2) == 1);
      design->setCanSeeName(query->getInt(3) == 1);
      design->setVisibleName(query->get(4));
      design->setCanSeeDescription(query->getInt(5) == 1);
      design->setVisibleDescription(query->get(6));
      design->setCanSeeNumExist(query->getInt(7) == 1);
      design->setVisibleNumExist(query->getInt(8));
      design->setCanSeeOwner(query->getInt(9) == 1);
      design->setVisibleOwner(query->getInt(10));
      design->setModTime(query->getU64(11));
    }

    querybuilder.str("");
    querybuilder << "SELECT componentid,quantity FROM playerdesignviewcomp WHERE playerid = " << playerid << " AND designid = " << designid << ";";
    design->setVisibleComponents(idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT propertyid,value FROM playerdesignviewprop WHERE playerid = " << playerid << " AND designid = " << designid << ";";
    singleQuery( querybuilder.str() );

    PropertyValue::Map pvlist;
    while(query->nextRow()){
      PropertyValue pv( query->getInt(0), 0.0);
      pv.setDisplayString(query->get(1));
      pvlist[pv.getPropertyId()] = pv;
    }
    design->setVisiblePropertyValues(pvlist);
    return design;
  } catch( MysqlException& ) {
    return DesignView::Ptr(); 
  }
}

bool MysqlPersistence::saveComponentView(uint32_t playerid, ComponentView::Ptr cv){
  try {
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM playercomponentview WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentviewcat WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentviewproperty WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    singleQuery( querybuilder.str() );

    querybuilder.str("");
    querybuilder << "INSERT INTO playercomponentview VALUES (" << playerid << ", " << cv->getComponentId() << ", ";
    querybuilder << ((cv->isCompletelyVisible()) ? 1 : 0) << ", " << ((cv->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(cv->getVisibleName()) << "', " << ((cv->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(cv->getVisibleDescription()) << "', " << ((cv->canSeeRequirementsFunc()) ? 1 : 0) << ", ";
    querybuilder << cv->getModTime() << ");";
    singleQuery( querybuilder.str() );

    insertSet( "playercomponentviewcat", playerid, cv->getComponentId(), cv->getVisibleCategories() );
    insertSet( "playercomponentviewproperty", playerid, cv->getComponentId(), cv->getVisiblePropertyFuncs() );

    return true;
  } catch( MysqlException& ) {
    return false; 
  }
}

ComponentView::Ptr MysqlPersistence::retrieveComponentView(uint32_t playerid, uint32_t componentid){
  try {
    ComponentView::Ptr comp;
    std::ostringstream querybuilder;
    {
      querybuilder << "SELECT * FROM playercomponentview WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
      MysqlQuery query( conn, querybuilder.str() );
      comp.reset( new ComponentView( componentid, query->getInt(2) == 1) );
      comp->setCanSeeName(query->getInt(3) == 1);
      comp->setVisibleName(query->get(4));
      comp->setCanSeeDescription(query->getInt(5) == 1);
      comp->setVisibleDescription(query->get(6));
      comp->setCanSeeRequirementsFunc(query->getInt(7) == 1);
      comp->setModTime(query->getU64(11));
    }

    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM playercomponentviewcat WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
    comp->setVisibleCategories(idSetQuery( querybuilder.str() ) );

    querybuilder.str("");
    querybuilder << "SELECT propertyid FROM playercomponentviewproperty WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
    comp->setVisiblePropertyFuncs(idSetQuery( querybuilder.str() ));
    return comp;
  } catch( MysqlException& ) {
    return ComponentView::Ptr(); 
  }
}

std::string MysqlPersistence::addslashes(const std::string& in) const{
  char* buf = new char[in.length() * 2 + 1];
  uint len = mysql_real_escape_string(conn, buf, in.c_str(), in.length());
  std::string rtv(buf, len);
  delete[] buf;
  return rtv;
}

uint32_t MysqlPersistence::getTableVersion(const std::string& name){
  try {
    return valueQuery( "SELECT version FROM tableversion WHERE name='" + addslashes(name) + "';");
  } catch( MysqlException& ) { 
    throw std::exception();
  }
}

bool MysqlPersistence::updatePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamposition WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparamposition VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << pob->getPosition().getX() << ", " << pob->getPosition().getY() << ", " << pob->getPosition().getZ() << ", " << pob->getRelative() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrievePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT posx,posy,posz,relative FROM objectparamposition WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  MysqlQuery query( conn, querybuilder.str() );
  pob->setPosition(Vector3d(strtoll(query->get(0), NULL, 10), strtoll(query->get(1), NULL, 10), strtoll(query->get(2), NULL, 10)));
  pob->setRelative(query->getInt(3));
  mysql_free_result(obresult);
  return true;
}

bool MysqlPersistence::updateVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamvelocity WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparamvelocity VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << vob->getVelocity().getX() << ", " << vob->getVelocity().getY() << ", " << vob->getVelocity().getZ() << ", " << vob->getRelative() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT velx,vely,velz,relative FROM objectparamvelocity WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  MysqlQuery query( conn, querybuilder.str() );
  vob->setPosition(Vector3d(strtoll(query->get(0), NULL, 10), strtoll(query->get(1), NULL, 10), strtoll(query->get(2), NULL, 10)));
  vob->setRelative(query->getInt(3));
  mysql_free_result(obresult);
  return true;
}

bool MysqlPersistence::updateOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamorderqueue WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparamorderqueue VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << oob->getQueueId() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT queueid FROM objectparamorderqueue WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  oob->setQueueId(valueQuery( querybuilder.str()));
  return true;
}

bool MysqlPersistence::updateResourceListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ResourceListObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamresourcelist WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = rob->getResources();
  querybuilder << "INSERT INTO objectparamresourcelist VALUES ";
  for(std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
      itcurr != reslist.end(); ++itcurr){
    if(itcurr != reslist.begin()){
      querybuilder << ", ";
    }
    querybuilder << "(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << itcurr->first << ", " << itcurr->second.first << ", " << itcurr->second.second << ")";
  }
  if(reslist.size() == 0){
    //fake resource to make sure there is something, removed when retreived.
    querybuilder << "(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", 0, 0, 0)";
  }
  querybuilder << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveResourceListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ResourceListObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT resid, available, possible FROM objectparamresourcelist WHERE objectid = " << objid;
  querybuilder << " AND turn = (SELECT MAX(turn) FROM objectparamresourcelist WHERE objectid = " << objid;
  querybuilder << " AND turn <= " << turn << " AND playerid = " << plid;
  querybuilder << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos;
  querybuilder << ") AND playerid = " << plid << " AND paramgroupid = " << pgroup;
  querybuilder << " AND paramgrouppos = " << pgpos << ";";
  MysqlQuery query( conn, querybuilder.str() );

  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist;
  while(query->nextRow()){
    uint32_t available = query->getInt(1);
    uint32_t possible = query->getInt(2);
    if(available != 0 || possible != 0){
      reslist[query->getInt(0)] = std::pair<uint32_t, uint32_t>(available, possible);
    }
  }
  rob->setResources(reslist);
  return true;
}

bool MysqlPersistence::updateReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamreference WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparamreference VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << rob->getReferenceType() << ", " << rob->getReferencedId() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT reftype, refval FROM objectparamreference WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  MysqlQuery query( conn, querybuilder.str() );
  rob->setReferenceType(query->getInt(0));
  rob->setReferencedId(query->getInt(1));
  return true;
}

bool MysqlPersistence::updateRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamrefquantitylist WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  std::map<std::pair<int32_t, uint32_t>, uint32_t> reflist = rob->getRefQuantityList();
  querybuilder << "INSERT INTO objectparamrefquantitylist VALUES ";
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = reflist.begin();
      itcurr != reflist.end(); ++itcurr){
    if(itcurr != reflist.begin()){
      querybuilder << ", ";
    }
    querybuilder << "(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << itcurr->first.first << ", " << itcurr->first.second << ", " << itcurr->second << ")";
  }
  if(reflist.size() == 0){
    //fake refquantity to make sure there is something, removed when retreived.
    querybuilder << "(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", 0, 0, 0)";
  }
  querybuilder << ";";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT reftype, refid, quant FROM objectparamrefquantitylist WHERE objectid = " << objid << " AND turn = (SELECT MAX(turn) FROM objectparamrefquantitylist WHERE objectid = " << objid;
  querybuilder << " AND turn <= " << turn << " AND playerid = " << plid;
  querybuilder << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos;
  querybuilder << ") AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  MysqlQuery query( conn, querybuilder.str() );
  std::map<std::pair<int32_t, uint32_t>, uint32_t> reflist;
  while(query->nextRow()){
    int32_t reftype = query->getInt(0);
    uint32_t refid = query->getInt(1);
    uint32_t quant = query->getInt(2);
    if(reftype != 0 && refid != 0 && quant != 0){
      reflist[std::pair<int32_t, uint32_t>(reftype, refid)] = quant;
    }
  }
  rob->setRefQuantityList(reflist);
  return true;
}

bool MysqlPersistence::updateIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparaminteger WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparaminteger VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << iob->getValue() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT val FROM objectparaminteger WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  iob->setValue( valueQuery( querybuilder.str()));
  return true;
}

bool MysqlPersistence::updateSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM objectparamsize WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
  singleQuery( querybuilder.str() );
  querybuilder.str("");
  querybuilder << "INSERT INTO objectparamsize VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << sob->getSize() << ");";
  singleQuery( querybuilder.str() );
  return true;
}

bool MysqlPersistence::retrieveSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT size FROM objectparamsize WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
  MysqlQuery query( conn, querybuilder.str() );
  sob->setSize(strtoll(query->get(0), NULL, 10));
  return true;
}

void MysqlPersistence::lock(){
  MysqlQuery::lock();
}

void MysqlPersistence::unlock(){
  MysqlQuery::unlock();
}

static void MysqlPersistence::idSetToStream( std::ostringstream& stream, const uint32_t id, const IdSet& idset ) const {
  for ( IdSet::const_iterator it = idset.begin(); it != idset.end(); ++it ) {
    if ( it != idset.begin() ) stream << ", ";
    stream << "(" << id << ", " << (*it) << ")";
  }
}

static void MysqlPersistence::idSetToStream( std::ostringstream& stream, const uint32_t id, const uint32_t id2, const IdSet& idset ) const {
  for ( IdSet::const_iterator it = idset.begin(); it != idset.end(); ++it ) {
    if ( it != idset.begin() ) stream << ", ";
    stream << "(" << id << ", " << id2 << ", " << (*it) << ")";
  }
}

static void idListToStream( std::ostringstream& stream, const uint32_t id, const IdList& idlist ) const {
  uint32_t slotnum = 0;
  for(IdList::const_iterator it = idlist.begin(); it != idlist.end(); ++it) {
    if ( it != idlist.begin() ) stream << ", ";
    stream << "(" << id << ", " << slotnum << ", " << (*it) << ")";
    slotnum++;
  }
}

static void MysqlPersistence::idMapToStream( std::ostringstream& stream, const uint32_t id, const IdMap& idmap ) const {
  for ( IdMap::const_iterator it = idmap.begin(); it != idmap.end(); ++it ) {
    if ( it != idmap.begin() ) stream << ", ";
    stream << "(" << id << ", " << it->first << ", " << it->second << ")";
  }
}

static void MysqlPersistence::idMapToStream( std::ostringstream& stream, const uint32_t id, const uint32_t id2, const IdMap& idmap ) const {
  for ( IdMap::const_iterator it = idmap.begin(); it != idmap.end(); ++it ) {
    if ( it != idmap.begin() ) stream << ", ";
    stream << "(" << id << ", " << id2 << ", " << it->first << ", " << it->second << ")";
  }
}

void  MysqlPersistence::insertSet ( const std::string& table, uint32_t id, const IdSet& idset ) {
  if (idset.empty()) return;
  std::ostringstream query;
  query << "INSERT INTO " << table << " VALUES ";
  idSetToStream( query, id, idset );
  query << ";";
  singleQuery( query.str() );
}

void  MysqlPersistence::insertSet ( const std::string& table, uint32_t id, uint32_t id2, const IdSet& idset ) {
  if (idset.empty()) return;
  std::ostringstream query;
  query << "INSERT INTO " << table << " VALUES ";
  idSetToStream( query, id, id2, idset );
  query << ";";
  singleQuery( query.str() );
}

void  MysqlPersistence::insertList( const std::string& table, uint32_t id, const IdList& idlist ) {
  if (idlist.empty()) return;
  std::ostringstream query;
  query << "INSERT INTO " << table << " VALUES ";
  idListToStream( query, id, idlist );
  query << ";";
  singleQuery( query.str() );
}

void  MysqlPersistence::insertMap ( const std::string& table, uint32_t id, const IdMap& idmap );
if (idmap.empty()) return;
std::ostringstream query;
query << "INSERT INTO " << table << " VALUES ";
idMapToStream( query, id, idmap );
query << ";";
singleQuery( query.str() );
}

void  MysqlPersistence::insertMap ( const std::string& table, uint32_t id, uint32_t id2, const IdMap& idmap );
if (idmap.empty()) return;
std::ostringstream query;
query << "INSERT INTO " << table << " VALUES ";
idMapToStream( query, id, id2, idmap );
query << ";";
singleQuery( query.str() );
}

void MysqlPersistence::singleQuery( const std::string& query ) {
  MysqlQuery q( conn, query );
}

uint32_t MysqlPersistence::valueQuery( const std::string& query ) {
  MysqlQuery q( conn, query );
  return q->getInt(0);
}

const IdSet& MysqlPersistence::idSetQuery( const std::string& query ) {
  MysqlQuery q( conn, query );
  IdSet set; 
  while(q->nextRow()){
    set.insert(q->getInt(0));
  }
  return set;
}

const IdList& MysqlPersistence::idListQuery( const std::string& query ) {
  MysqlQuery q( conn, query );
  IdList list;
  while(q->nextRow()){
    list.push_back(q->getInt(0));
  }
  return list;
}

const IdMap& MysqlPersistence::idMapQuery( const std::string& query ) {
  MysqlQuery q( conn, query );
  IdMap map;
  while(q->nextRow()){
    map[ q->getInt(0) ] = q->getInt(0);
  }
  return map;
}


  MysqlQuery::MysqlQuery( MYSQL *conn, const std::string& new_query ) 
: connection( conn ), result( NULL ), row( NULL ), query( new_query ) 
{
  lock();
  if ( mysql_query( conn, query.c_str() ) != 0 ) {
    unlock(); // Destructor WON'T get called if throw is in constructor 
    throw MysqlException( "Query '"+query+"' failed!");
  }
}

  const std::string& MysqlQuery::get( uint32_t index ) {
    if ( result == NULL ) 
    {
      fetchResult();
      nextRow();
    }
    if ( row == NULL ) {
      throw MysqlException( "Query '"+query+"' row empty!");
    }
    return row[index];
  }

int MysqlQuery::getInt( uint32_t index ) {
  if ( result == NULL ) {
    fetchResult();
    nextRow();
  }
  if ( row == NULL ) {
    throw MysqlException( "Query '"+query+"' row empty!");
  }
  return atoi(row[index]);
}

uint64_t MysqlQuery::getU64( uint32_t index ) {
  if ( result == NULL ) {
    fetchResult();
    nextRow();
  }
  if ( row == NULL ) {
    throw MysqlException( "Query '"+query+"' row empty!");
  }
  return strtoull(row[index],NULL,10);
}

MysqlQuery::fetchResult() {
  result = mysql_store_result(connection);
  if ( result == NULL ) {
    throw MysqlException( "Query '"+query+"' result failed!");
  }
  unlock(); 
}

bool MysqlQuery::validRow() {
  if ( result == NULL ) {
    fetchResult();
    nextRow();
  }
  return row != NULL;
}

bool MysqlQuery::nextRow() {
  if ( result == NULL ) fetchResult();
  row = mysql_fetch_row(result);
  return row != NULL;
}

MysqlQuery::~MysqlQuery() {
  if ( result != NULL ) mysql_free_result(result);
  unlock(); // unlock needs to be multiunlock safe
}

