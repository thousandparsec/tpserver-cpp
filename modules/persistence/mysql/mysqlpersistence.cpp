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

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

#include <sstream>

#include <tpserver/logging.h>
#include <tpserver/settings.h>
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>

#include <tpserver/object.h>
#include <tpserver/orderqueue.h>
#include <tpserver/order.h>
#include <tpserver/orderparameter.h>
#include <tpserver/listparameter.h>
#include <tpserver/objectorderparameter.h>
#include <tpserver/spacecoordparam.h>
#include <tpserver/stringparameter.h>
#include <tpserver/timeparameter.h>
#include <tpserver/board.h>
#include <tpserver/message.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/category.h>
#include <tpserver/design.h>
#include <tpserver/component.h>
#include <tpserver/property.h>


#include "mysqlpersistence.h"

extern "C" {
  #define tp_init libtpmysql_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setPersistence(new MysqlPersistence());
  }
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
            if(mysql_query(conn, "INSERT INTO tableversion VALUES (NULL, 'tableversion', 1), (NULL, 'gameinfo', 0), "
                    "(NULL, 'object', 0), (NULL, 'orderqueue', 0), (NULL, 'orderqueueowner', 0)"
                    "(NULL, 'ordertype', 0), (NULL, 'orderresource', 0), (NULL, 'orderslot', 0), "
                    "(NULL, 'orderparamspace', 0), (NULL, 'orderparamobject', 0), "
                    "(NULL, 'orderparamstring', 0), (NULL, 'orderparamtime', 0), "
                    "(NULL, 'orderparamlist', 0), "
                    "(NULL, 'board', 0), (NULL, 'message', 0), (NULL, 'messagereference', 0), (NULL, 'messageslot', 0), "
                    "(NULL, 'player', 0), (NULL, 'playerscore', 0), (NULL, 'playerdesignview', 0), (NULL, 'playerdesignusable', 0), (NULL, 'playercomponentview', 0), "
                    "(NULL, 'playercomponentusable', 0), (NULL, 'playerobjectview', 0), (NULL, 'playerobjectowned', 0), "
                    "(NULL, 'category', 0), (NULL, 'design',0), (NULL, 'designcomponent', 0), (NULL, 'designproperty', 0), "
                    "(NULL, 'component', 0), (NULL, 'componentcat', 0), (NULL, 'componentproperty', 0), "
                    "(NULL, 'property', 0), (NULL, 'propertycat', 0), (NULL, 'resourcedesc', 0);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE gameinfo (metakey VARCHAR(50) NOT NULL, ctime BIGINT UNSIGNED NOT NULL PRIMARY KEY, turnnum INT UNSIGNED NOT NULL);") != 0){
              throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE object (objectid INT UNSIGNED NOT NULL, turnnum INT UNSIGNED NOT NULL, type INT UNSIGNED NOT NULL, " 
                    "name TEXT NOT NULL, desc TEXT NOT NULL, parentid INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY(objectid, turnnum));") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE orderqueue (queueid INT UNSIGNED NOT NULL, objectid INT UNSIGNED NOT NULL, active TINYINT NOT NULL, repeating TINYINT NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
              throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE orderqueueowner (queueid INT UNSIGNED NOT NULL, playerid INT UNSIGNED NOT NULL);") != 0){
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
                        "orderid INT UNSIGNED NOT NULL UNIQUE, PRIMARY KEY (queueid, slot));") != 0){
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
                    "visname TEXT NOT NULL, descvisible TINYINT NOT NULL, visdesc TEXT NOT NULL, existvisible TINYINT NOT NULL, visexist INT UNSIGNED NOT NULL, ownervisible TINYINT NOT NULL, visowner INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL, PRIMARY KEY (playerid, designid, turnnum));") != 0){
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
            if(mysql_query(conn, "CREATE TABLE property (propertyid INT UNSIGNED NOT NULL PRIMARY KEY, categoryid INT UNSIGNED NOT NULL,"
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
  std::ostringstream querybuilder;
  Game* game = Game::getGame();
  querybuilder << "INSERT INTO gameinfo VALUES ('" << addslashes(game->getKey()) << "', ";
  querybuilder << game->getGameStartTime() << ", " << game->getTurnNumber() << ");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not save gameinfo - %s", mysql_error(conn));
    unlock();
    return false;
  }
  unlock();
  return true;;
}

bool MysqlPersistence::retrieveGameInfo(){
  lock();
  if(mysql_query(conn, "SELECT * FROM gameinfo;") != 0){
    Logger::getLogger()->error("Mysql: Could not retrieve gameinfo - %s", mysql_error(conn));
    unlock();
    return false;
  }
  MYSQL_RES *giresult = mysql_store_result(conn);
  if(giresult == NULL){
    Logger::getLogger()->error("Mysql: retrieve gameinfo: Could not store result - %s", mysql_error(conn));
    unlock();
    return false;
  }
  unlock();
  
  MYSQL_ROW row = mysql_fetch_row(giresult);
  if(row == NULL){
    Logger::getLogger()->warning("Mysql: No existing gameinfo");
    mysql_free_result(giresult);
    return false;
  }
  
  Game* game = Game::getGame();
  
  game->setKey(row[0]);
  game->setGameStartTime(strtoull(row[1], NULL, 10));
  game->setTurnNumber(atoi(row[2]));
  
  mysql_free_result(giresult);
  
  return true;
}

bool MysqlPersistence::saveObject(IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO object VALUES (" << ob->getID() << ", " << ob->getType() << ", ";
    querybuilder << "'" << addslashes(ob->getName()) << "', '" << addslashes(ob->getDescription()) << "', ";
    querybuilder << ob->getParent() << ", " << ob->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store object %d - %s", ob->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    
    //TODO objectparameters
    
    ob->setDirty(!rtv);
    unlock();
    return rtv;
}

//#include <iostream>

bool MysqlPersistence::updateObject(IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE object SET type=" << ob->getType() << ", name='";
    querybuilder << addslashes(ob->getName()) << "', description='", addslashes(ob->getDescription());
    querybuilder << "', parentid=" << ob->getParent() << ", modtime=" << ob->getModTime() << " WHERE objectid=" << ob->getID() << ";";
    lock();
    std::string query = querybuilder.str();
    //std::cout << "Query: " << query << std::endl;
    if(mysql_query(conn, query.c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update object %d - %s", ob->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    
    //TODO objectparameters
    
    ob->setDirty(!rtv);
    unlock();
    return rtv;
}

IGObject* MysqlPersistence::retrieveObject(uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM object WHERE objectid = " << obid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve object %d - %s", obid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve object: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    
    //children object ids
    querybuilder.str("");
    querybuilder << "SELECT objectid FROM object WHERE parentid = " << obid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve object %d - %s", obid, mysql_error(conn));
        unlock();
        mysql_free_result(obresult);
        return NULL;
    }
    MYSQL_RES *childres = mysql_store_result(conn);

    if(childres == NULL){
        Logger::getLogger()->error("Mysql: retrieve object: Could not store result - %s", mysql_error(conn));
        unlock();
        mysql_free_result(obresult);
        return NULL;
    }
    
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such object %d", obid);
        mysql_free_result(obresult);
        mysql_free_result(childres);
        return NULL;
    }
    IGObject* object = new IGObject();
    object->setID(obid);
    object->setType(atoi(row[1]));
    object->setName(row[2]);
    object->setDescription(row[3]);
    object->setParent(atoi(row[4]));
    
    MYSQL_ROW children;
    while((children = mysql_fetch_row(childres)) != NULL){
        uint32_t childid = atoi(children[0]);
        Logger::getLogger()->debug("childid: %d", childid);
        if(childid != object->getID())
            object->addContainedObject(childid);
    }
    Logger::getLogger()->debug("num children: %d", object->getContainedObjects().size());
    mysql_free_result(childres);

    // fetch type-specific information
    
    //TODO objectparameters
    
    object->setModTime(strtoull(row[5], NULL, 10));
    object->setDirty(false);
    
    mysql_free_result(obresult);
    return object;
}

bool MysqlPersistence::removeObject(uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM object WHERE objectid=" << obid <<";";
    uint32_t objecttype;
    lock();
    try{
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: remove object query error: %s", mysql_error(conn));
            throw std::exception();
        }else{
            MYSQL_RES *objtyperes = mysql_store_result(conn);
            if(objtyperes == NULL){
                Logger::getLogger()->error("Mysql: remove object query result error: %s", mysql_error(conn));
                throw std::exception();
            }
            unlock();
    
            MYSQL_ROW row = mysql_fetch_row(objtyperes);
            if(row == NULL || row[0] == NULL){ 
                Logger::getLogger()->warning("Mysql remove object: object not found");
                throw std::exception();
            }
            objecttype = atoi(row[0]);
            mysql_free_result(objtyperes);
        }
    }catch(std::exception e){
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM object WHERE objectid=" << obid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove object %d - %s", obid, mysql_error(conn));
        unlock();
        return false;
    }
    bool rtv;
    //store type-specific information
    
    //TODO objectparameters
    
    unlock();
    return rtv;
    
}

uint32_t MysqlPersistence::getMaxObjectId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(objectid) FROM object;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max object id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max objectid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getObjectIds(){
    lock();
    if(mysql_query(conn, "SELECT objectid FROM object;") != 0){
        Logger::getLogger()->error("Mysql: Could not query object ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get objectids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveOrderQueue(const OrderQueue* oq){
  std::ostringstream querybuilder;
  querybuilder << "INSERT INTO orderqueue VALUES (" << oq->getQueueId() << ", " << oq->getObjectId() << ", ";
  querybuilder << (oq->isActive() ? 1 : 0) << ", " << (oq->isRepeating() ? 1 : 0) << ", " << oq->getModTime() << ");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not insert orderqueue %d - %s", oq->getQueueId(), mysql_error(conn));
    unlock();
    return false;
  }
  unlock();
  std::list<uint32_t> list = oq->getOrderSlots();
  if(!list.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderslot VALUES ";
    uint32_t slotnum = 0;
    for(std::list<uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
      if(itcurr != list.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << slotnum << ", " << (*itcurr) << ")";
      slotnum++;
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderslots - %s", mysql_error(conn));
      unlock();
      return false;
    }
    unlock();
  }
  std::set<uint32_t> owners = oq->getOwner();
  if(!owners.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderqueueowner VALUES ";
    for(std::set<uint32_t>::iterator itcurr = owners.begin(); itcurr != owners.end(); ++itcurr){
      if(itcurr != owners.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << (*itcurr) << ")";
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderqueue owners - %s", mysql_error(conn));
      unlock();
      return false;
    }
    unlock();
  }
  return true;
}

bool MysqlPersistence::updateOrderQueue(const OrderQueue* oq){
  std::ostringstream querybuilder;
  querybuilder << "UPDATE orderqueue set objectid=" << oq->getObjectId() << ", active=" << (oq->isActive() ? 1 : 0);
  querybuilder << ", repeating=" << (oq->isRepeating() ? 1 : 0) << ", modtime=" << oq->getModTime()<< " WHERE queueid=" << oq->getQueueId() << ");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not update orderqueue %d - %s", oq->getQueueId(), mysql_error(conn));
    unlock();
    return false;
  }
  
  querybuilder.str("");
  querybuilder << "DELETE FROM orderslot WHERE queueid=" << oq->getQueueId() <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not remove orderslots for orderqueue %d - %s", oq->getQueueId(), mysql_error(conn));
    unlock();
    return false;
  }
  querybuilder.str("");
  querybuilder << "DELETE FROM orderqueueowner WHERE queueid=" << oq->getQueueId() <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not remove owners for orderqueue %d - %s", oq->getQueueId(), mysql_error(conn));
    unlock();
    return false;
  }
  unlock();
  
  std::list<uint32_t> list = oq->getOrderSlots();
  if(!list.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderslot VALUES ";
    uint32_t slotnum = 0;
    for(std::list<uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
      if(itcurr != list.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << slotnum << ", " << (*itcurr) << ")";
      slotnum++;
    }
    querybuilder << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderslots - %s", mysql_error(conn));
      return false;
    }
  }
  std::set<uint32_t> owners = oq->getOwner();
  if(!owners.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderqueueowner VALUES ";
    for(std::set<uint32_t>::iterator itcurr = owners.begin(); itcurr != owners.end(); ++itcurr){
      if(itcurr != owners.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << (*itcurr) << ")";
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderqueue owners - %s", mysql_error(conn));
      unlock();
      return false;
    }
    unlock();
  }
  
  return true;
}

OrderQueue* MysqlPersistence::retrieveOrderQueue(uint32_t oqid){
  std::ostringstream querybuilder;
  querybuilder << "SELECT * FROM orderqueue WHERE queueid=" << oqid << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: get orderqueue: Could not store result - %s", mysql_error(conn));
    unlock();
    return NULL;
  }
  MYSQL_RES *oqresult = mysql_store_result(conn);
  if(oqresult == NULL){
    Logger::getLogger()->error("Mysql: get orderqueue: Could not store result - %s", mysql_error(conn));
    unlock();
    return NULL;
  }
  querybuilder.str("");
  querybuilder << "SELECT orderid FROM orderslot WHERE queueid=" << oqid <<" ORDER BY slot;";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not query order list - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      return NULL;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: get order list: Could not store result - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      return NULL;
  }
  querybuilder.str("");
  querybuilder << "SELECT playerid FROM orderqueueowner WHERE queueid=" << oqid <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not query orderqueue owners - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      mysql_free_result(ordresult);
      return NULL;
  }
  MYSQL_RES *ownerresult = mysql_store_result(conn);
  if(ownerresult == NULL){
      Logger::getLogger()->error("Mysql: get order queue owners: Could not store result - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      mysql_free_result(ordresult);
      return NULL;
  }
  unlock();
  
  
  MYSQL_ROW oqrow = mysql_fetch_row(oqresult);
  if(oqrow == NULL){
    Logger::getLogger()->error("Mysql: get orderqueue: No such orderqueue - %d", oqid);
    mysql_free_result(oqresult);
    mysql_free_result(ordresult);
    mysql_free_result(ownerresult);
    return NULL;
  }
  
  OrderQueue* oq = new OrderQueue();
  oq->setQueueId(oqid);
  oq->setObjectId(atoi(oqrow[1]));
  oq->setActive(atoi(oqrow[2]) == 1);
  oq->setRepeating(atoi(oqrow[3]) == 1);
  oq->setModTime(strtoull(oqrow[4], NULL, 10));
  
  mysql_free_result(oqresult);
  
  uint32_t max = 0;
  MYSQL_ROW olrow;
  std::list<uint32_t> oolist;
  while((olrow = mysql_fetch_row(ordresult)) != NULL){
    uint32_t olvalue = atoi(olrow[0]);
    oolist.push_back(olvalue);
    if(olvalue > max)
      max = olvalue;
  }
  mysql_free_result(ordresult);
  oq->setOrderSlots(oolist);
  oq->setNextOrderId(max+1);
  
  MYSQL_ROW ownrow;
  std::set<uint32_t> owners;
  while((ownrow = mysql_fetch_row(ownerresult)) != NULL){
    owners.insert(atoi(ownrow[0]));
  }
  mysql_free_result(ownerresult);
  oq->setOwners(owners);
  
  
  return oq;
}

bool MysqlPersistence::removeOrderQueue(uint32_t oqid){
  std::ostringstream querybuilder;
  
  
  querybuilder << "DELETE FROM orderslot WHERE queueid=" << oqid <<";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove orderslots for object %d - %s", oqid, mysql_error(conn));
      unlock();
      return false;
  }
  querybuilder.str("");
  querybuilder << "DELETE FROM orderqueueowner WHERE queueid=" << oqid <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not remove owners for orderqueue %d - %s", oqid, mysql_error(conn));
    unlock();
    return false;
  }
  unlock();
  return true;
}

std::set<uint32_t> MysqlPersistence::getOrderQueueIds(){
  lock();
  if(mysql_query(conn, "SELECT queueid FROM orderqueue;") != 0){
    Logger::getLogger()->error("Mysql: Could not query orderqueue ids - %s", mysql_error(conn));
    unlock();
    return std::set<uint32_t>();
  }
  MYSQL_RES *oqresult = mysql_store_result(conn);
  unlock();
  if(oqresult == NULL){
    Logger::getLogger()->error("Mysql: get orderqueueids: Could not store result - %s", mysql_error(conn));
    return std::set<uint32_t>();
  }
  MYSQL_ROW max;
  std::set<uint32_t> vis;
  while((max = mysql_fetch_row(oqresult)) != NULL){
    vis.insert(atoi(max[0]));
  }
  mysql_free_result(oqresult);
  return vis;
}

uint32_t MysqlPersistence::getMaxOrderQueueId(){
  lock();
  if(mysql_query(conn, "SELECT MAX(queueid) FROM orderqueue;") != 0){
    Logger::getLogger()->error("Mysql: Could not query max orderqueue id - %s", mysql_error(conn));
    unlock();
    return 0;
  }
  MYSQL_RES *oqresult = mysql_store_result(conn);
  unlock();
  if(oqresult == NULL){
    Logger::getLogger()->error("Mysql: get max orderqueueid: Could not store result - %s", mysql_error(conn));
    return 0;
  }
  MYSQL_ROW max = mysql_fetch_row(oqresult);
  uint32_t maxid = 0;
  if(max[0] != NULL){
    maxid = atoi(max[0]);
  }
  mysql_free_result(oqresult);
  return maxid;
}
    
bool MysqlPersistence::saveOrder(uint32_t queueid, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO ordertype VALUES (" << queueid << ", " << ordid << ", " << ord->getType() << ", " << ord->getTurns() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store order %d,%d - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    //store resources
    std::map<uint32_t, uint32_t> ress = ord->getResources();
    if(!ress.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO orderresource VALUES ";
      for(std::map<uint32_t, uint32_t>::iterator itcurr = ress.begin(); itcurr != ress.end(); ++itcurr){
        if(itcurr != ress.begin())
          querybuilder << ", ";
        querybuilder << "(" << queueid << ", " << ordid << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store order %d,%d resources - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
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
    
    unlock();
    return true;
  
}

bool MysqlPersistence::updateOrder(uint32_t queueid, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE ordertype SET type = " << ord->getType() << ", turns=" << ord->getTurns() << " WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
     lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update order %d,%d - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    //update resources
    querybuilder.str("");
    querybuilder << "DELETE FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update (a) order %d,%d resources - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
      }
    unlock();
    std::map<uint32_t, uint32_t> ress = ord->getResources();
    if(!ress.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO orderresource VALUES ";
      for(std::map<uint32_t, uint32_t>::iterator itcurr = ress.begin(); itcurr != ress.end(); ++itcurr){
        if(itcurr != ress.begin())
          querybuilder << ", ";
        querybuilder << "(" << queueid << ", " << ordid << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update (b) order %d,%d resources - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
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
}

Order* MysqlPersistence::retrieveOrder(uint32_t queueid, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type,turns FROM ordertype WHERE queueid = " << queueid << " AND orderid = " << ordid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve order %d,%d - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *ordresult = mysql_store_result(conn);
    if(ordresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve order: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock();
    MYSQL_ROW row = mysql_fetch_row(ordresult);
    uint32_t ordertype;
    uint32_t turns;
    if(row != NULL && row[0] != NULL && row[1] != NULL){
        ordertype = atoi(row[0]);
        turns = atoi(row[1]);
    }else{
        mysql_free_result(ordresult);
        Logger::getLogger()->error("Mysql: retrieve order: no such order %d,%d - %s", queueid, ordid, mysql_error(conn));
        return NULL;
   }
    mysql_free_result(ordresult);
    Order* order = Game::getGame()->getOrderManager()->createOrder(ordertype);
    order->setTurns(turns);
    //fetch resources
    querybuilder.str("");
    querybuilder << "SELECT resourceid, amount FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve order resources %d,%d - %s", queueid, ordid, mysql_error(conn));
        unlock();
        delete order;
        return NULL;
    }
    ordresult = mysql_store_result(conn);
    if(ordresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve order resources: Could not store result - %s", mysql_error(conn));
        unlock();
        delete order;
        return NULL;
    }
    unlock();
    
    while((row = mysql_fetch_row(ordresult)) != NULL){
        order->addResource(atoi(row[0]), atoi(row[1]));
    }
    mysql_free_result(ordresult);
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
}

bool MysqlPersistence::removeOrder(uint32_t queueid, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT type FROM ordertype WHERE queueid=" << queueid << " AND orderid=" << ordid <<";";
    uint32_t ordertype;
    lock();
    try{
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: remove order query error: %s", mysql_error(conn));
            throw std::exception();
        }else{
            MYSQL_RES *ordtyperes = mysql_store_result(conn);
            if(ordtyperes == NULL){
                Logger::getLogger()->error("Mysql: remove order query result error: %s", mysql_error(conn));
                throw std::exception();
            }
    
            MYSQL_ROW row = mysql_fetch_row(ordtyperes);
            if(row == NULL || row[0] == NULL){ 
                Logger::getLogger()->warning("Mysql remove order: order not found");
                throw std::exception();
            }
            ordertype = atoi(row[0]);
            mysql_free_result(ordtyperes);
        }
    }catch(std::exception e){
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM ordertype WHERE queueid= " << queueid << " AND orderid=" << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove order %d,%d - %s", queueid, ordid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    //remove resources
    querybuilder.str("");
    querybuilder << "DELETE FROM orderresource WHERE queueid=" << queueid << " AND orderid=" << ordid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove order %d,%d resources - %s", queueid, ordid, mysql_error(conn));
      unlock();
      return false;
    }
    unlock();
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
}

bool MysqlPersistence::updateSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamspace WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove/update SpaceCoordParam %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamspace VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << scp->getPosition().getX() << ", " << scp->getPosition().getY() << ", ";
  querybuilder << scp->getPosition().getZ() <<");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store/update SpaceCoordParam %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::retrieveSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos, SpaceCoordParam* scp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT posx,posy,posz FROM orderparamspace WHERE queueid = " << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve SpaceCoordParam %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve SpaceCoordParam: Could not store result - %s", mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  MYSQL_ROW row = mysql_fetch_row(ordresult);
  if(row == NULL){
      mysql_free_result(ordresult);
      Logger::getLogger()->error("Mysql: retrieve SpaceCoordParam: no such parameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      return false;
  }
  
  scp->setPosition(Vector3d(atoll(row[0]), atoll(row[1]), atoll(row[2])));
  
  mysql_free_result(ordresult);
  
  return true;
}

bool MysqlPersistence::removeSpaceCoordParam(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamspace WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove SpaceCoordParam %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::updateListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove/update ListParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  std::map<uint32_t, uint32_t> list = lp->getList();
  if(!list.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderparamlist VALUES ";
    for(std::map<uint32_t, uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
      if(itcurr != list.begin())
        querybuilder << ", ";
      querybuilder << "(" << queueid << ", " << ordid << ", " << pos << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store/update ListParameter %d,%d,&d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
    }
    unlock();
  }
  return true;
}

bool MysqlPersistence::retrieveListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ListParameter* lp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT listid, amount FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position = " << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve ListParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve ListParameter: Could not store result - %s", mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  
  std::map<uint32_t, uint32_t> list;
  MYSQL_ROW row;
  while((row = mysql_fetch_row(ordresult)) != NULL){
      list[atoi(row[0])] = atoi(row[1]);
  }
  mysql_free_result(ordresult);
  lp->setList(list);
  return true;
}

bool MysqlPersistence::removeListParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamlist WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove ListParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::updateObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamobject WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove/update ObjectOrderParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamobject VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << ob->getObjectId() << ");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store/update ObjectOrderParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::retrieveObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, ObjectOrderParameter* ob){
  std::ostringstream querybuilder;
  querybuilder << "SELECT objectid FROM orderparamobject WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve ObjectOrderParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve ObjectOrderParameter: Could not store result - %s", mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  MYSQL_ROW row = mysql_fetch_row(ordresult);
  if(row == NULL){
      mysql_free_result(ordresult);
      Logger::getLogger()->error("Mysql: retrieve ObjectOrderParameter: no such parameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      return false;
  }
  
  ob->setObjectId(atoi(row[0]));
  
  mysql_free_result(ordresult);
  
  return true;
}

bool MysqlPersistence::removeObjectOrderParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamobject WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove ObjectOrderParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::updateStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamstring WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove/update StringParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamstring VALUES (" << queueid << ", " << ordid << ", " << pos << ", '";
  querybuilder << addslashes(st->getString()) <<"');";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store/update StringParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::retrieveStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, StringParameter* st){
  std::ostringstream querybuilder;
  querybuilder << "SELECT thestring FROM orderparamstring WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve StringParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve StringParameter: Could not store result - %s", mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  MYSQL_ROW row = mysql_fetch_row(ordresult);
  if(row == NULL){
      mysql_free_result(ordresult);
      Logger::getLogger()->error("Mysql: retrieve StringParameter: no such parameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      return false;
  }
  
  st->setString(std::string(row[0]));
  
  mysql_free_result(ordresult);
  
  return true;
}

bool MysqlPersistence::removeStringParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamstring WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove StringParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::updateTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamtime WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove/update TimeParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  querybuilder.str("");
  querybuilder << "INSERT INTO orderparamtime VALUES (" << queueid << ", " << ordid << ", " << pos << ", ";
  querybuilder << tp->getTime() <<");";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store/update TimeParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::retrieveTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos, TimeParameter* tp){
  std::ostringstream querybuilder;
  querybuilder << "SELECT turns FROM orderparamtime WHERE queueid=" << queueid << " AND orderid = " << ordid << " AND position = " << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve TimeParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  MYSQL_RES *ordresult = mysql_store_result(conn);
  if(ordresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve TimeParameter: Could not store result - %s", mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  MYSQL_ROW row = mysql_fetch_row(ordresult);
  if(row == NULL){
      mysql_free_result(ordresult);
      Logger::getLogger()->error("Mysql: retrieve TimeParameter: no such parameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      return false;
  }
  
  tp->setTime(atoi(row[0]));
  
  mysql_free_result(ordresult);
  
  return true;
}
  
bool MysqlPersistence::removeTimeParameter(uint32_t queueid, uint32_t ordid, uint32_t pos){
  std::ostringstream querybuilder;
  querybuilder << "DELETE FROM orderparamtime WHERE queueid=" << queueid << " AND orderid=" << ordid << " AND position=" << pos << ";";
  lock();
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not remove TimeParameter %d,%d,%d - %s", queueid, ordid, pos, mysql_error(conn));
      unlock();
      return false;
  }
  unlock();
  return true;
}

bool MysqlPersistence::saveBoard(Board* board){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO board VALUES (" << board->getBoardID() << ", '" << addslashes(board->getName()) << "', '";
    querybuilder << addslashes(board->getDescription()) << "', " << board->getNumMessages() << ", ";
    querybuilder << board->getModTime() <<");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store board %d - %s", board->getBoardID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}

bool MysqlPersistence::updateBoard(Board* board){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE board SET name='" << addslashes(board->getName()) << "', description='" << addslashes(board->getDescription());
    querybuilder << "', nummessages=" << board->getNumMessages() << ", modtime=" << board->getModTime();
    querybuilder << " WHERE boardid=" << board->getBoardID() << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update board %d - %s", board->getBoardID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}

Board* MysqlPersistence::retrieveBoard(uint32_t boardid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM board WHERE boardid = " << boardid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve board %d - %s", boardid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve board: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such board %d", boardid);
        mysql_free_result(obresult);
        return NULL;
    }
    Board* board = new Board();
    board->setBoardID(boardid);
    board->setName(row[1]);
    board->setDescription(row[2]);
    board->setNumMessages(atoi(row[3]));
    board->setModTime(strtoull(row[4], NULL, 10));
    mysql_free_result(obresult);
    return board;
}

uint32_t MysqlPersistence::getMaxBoardId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(boardid) FROM board;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max board id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max boardid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getBoardIds(){
    lock();
    if(mysql_query(conn, "SELECT boardid FROM board;") != 0){
        Logger::getLogger()->error("Mysql: Could not query board ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get boardids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveMessage(uint32_t msgid, Message* msg){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO message VALUES (" << msgid << ", '" << addslashes(msg->getSubject()) << "', '";
    querybuilder << addslashes(msg->getBody()) << "', " << msg->getTurn() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store message %d - %s", msgid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    std::set<std::pair<int32_t, uint32_t> > refs = msg->getReferences();
    if(!refs.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO messagereference VALUES ";
        for(std::set<std::pair<int32_t, uint32_t> >::iterator itcurr = refs.begin(); itcurr != refs.end(); ++itcurr){
            if(itcurr != refs.begin())
                querybuilder << ", ";
            querybuilder << "(" << msgid << ", " << (*itcurr).first << ", " << (*itcurr).second << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store message references %d - %s", msgid, mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

Message* MysqlPersistence::retrieveMessage(uint32_t msgid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM message WHERE messageid = " << msgid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve message %d - %s", msgid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *msgresult = mysql_store_result(conn);
    if(msgresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve message: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock();
    MYSQL_ROW row = mysql_fetch_row(msgresult);
    if(row == NULL){
        mysql_free_result(msgresult);
        Logger::getLogger()->error("Mysql: retrieve message: no such message %d - %s", msgid, mysql_error(conn));
        return NULL;
    }
    Message* msg = new Message();
    msg->setSubject(row[1]);
    msg->setBody(row[2]);
    msg->setTurn(atoi(row[3]));
    mysql_free_result(msgresult);
    
    querybuilder.str("");
    querybuilder << "SELECT type,refid FROM messagereference WHERE messageid = " << msgid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve message references %d - %s", msgid, mysql_error(conn));
        unlock();
        delete msg;
        return NULL;
    }
    msgresult = mysql_store_result(conn);
    if(msgresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve message references: Could not store result - %s", mysql_error(conn));
        unlock();
        delete msg;
        return NULL;
    }
    unlock();
    
    while((row = mysql_fetch_row(msgresult)) != NULL){
        msg->addReference(atoi(row[0]), atoi(row[1]));
    }
    mysql_free_result(msgresult);
    
    return msg;
}

bool MysqlPersistence::removeMessage(uint32_t msgid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM message WHERE messageid=" << msgid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove message %d - %s", msgid, mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM messagereference WHERE messageid=" << msgid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove message references %d - %s", msgid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}

bool MysqlPersistence::saveMessageList(uint32_t bid, std::list<uint32_t> list){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM messageslot WHERE boardid=" << bid <<";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove messageslots for board %d - %s", bid, mysql_error(conn));
        unlock();
        return false;
    }
    if(!list.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO messageslot VALUES ";
        uint32_t slotnum = 0;
        for(std::list<uint32_t>::iterator itcurr = list.begin(); itcurr != list.end(); ++itcurr){
            if(itcurr != list.begin()){
                querybuilder << ", ";
            }
            querybuilder << "(" << bid << ", " << slotnum << ", " << (*itcurr) << ")";
            slotnum++;
        }
        querybuilder << ";";
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store messageslots - %s", mysql_error(conn));
            return false;
        }
    }
    return true;
}

std::list<uint32_t> MysqlPersistence::retrieveMessageList(uint32_t bid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT messageid FROM messageslot WHERE boardid=" << bid <<" ORDER BY slot;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not query message list - %s", mysql_error(conn));
        unlock();
        return std::list<uint32_t>();
    }
    MYSQL_RES *ordresult = mysql_store_result(conn);
    unlock();
    if(ordresult == NULL){
        Logger::getLogger()->error("Mysql: get message list: Could not store result - %s", mysql_error(conn));
        return std::list<uint32_t>();
    }
    MYSQL_ROW max;
    std::list<uint32_t> bmlist;
    while((max = mysql_fetch_row(ordresult)) != NULL){
        bmlist.push_back(atoi(max[0]));
    }
    mysql_free_result(ordresult);

    return bmlist;
}

uint32_t MysqlPersistence::getMaxMessageId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(messageid) FROM message;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max message id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max messageid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

bool MysqlPersistence::saveResource(ResourceDescription* res){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO resourcedesc VALUES (" << res->getResourceType() << ", '" << addslashes(res->getNameSingular()) << "', '";
    querybuilder << addslashes(res->getNamePlural()) << "', '" << addslashes(res->getUnitSingular()) << "', '";
    querybuilder << addslashes(res->getUnitPlural()) << "', '" << addslashes(res->getDescription()) << "', ";
    querybuilder << res->getMass() << ", " << res->getVolume() << ", " << res->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store resource %d - %s", res->getResourceType(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}

ResourceDescription* MysqlPersistence::retrieveResource(uint32_t restype){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM resourcedesc WHERE resourcetype = " << restype << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve resource %d - %s", restype, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve resource: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such resource %d", restype);
        mysql_free_result(obresult);
        return NULL;
    }
    ResourceDescription *res = new ResourceDescription();
    res->setResourceType(restype);
    res->setNameSingular(row[1]);
    res->setNamePlural(row[2]);
    res->setUnitSingular(row[3]);
    res->setUnitPlural(row[4]);
    res->setDescription(row[5]);
    res->setMass(atoi(row[6]));
    res->setVolume(atoi(row[7]));
    res->setModTime(strtoull(row[8], NULL, 10));
    mysql_free_result(obresult);
    return res;
}

uint32_t MysqlPersistence::getMaxResourceId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(resourcetype) FROM resourcedesc;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max resource id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max resourceid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getResourceIds(){
    lock();
    if(mysql_query(conn, "SELECT resourcetype FROM resourcedesc;") != 0){
        Logger::getLogger()->error("Mysql: Could not query resourcetypes - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get resourceids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::savePlayer(Player* player){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO player VALUES (" << player->getID() << ", '" << addslashes(player->getName()) << "', '";
    querybuilder << addslashes(player->getPass()) << "', '" << addslashes(player->getEmail()) << "', '";
    querybuilder << addslashes(player->getComment()) << "', " << player->getBoardId() << ", ";
    querybuilder << ((player->isAlive()) ? 1 : 0) << ", " << player->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store player %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    //save scores
    std::map<uint32_t, uint32_t> scores = player->getAllScores();
    if(!scores.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerscore VALUES ";
        for(std::map<uint32_t, uint32_t>::iterator itcurr = scores.begin(); itcurr != scores.end(); ++itcurr){
            if(itcurr != scores.begin()){
                querybuilder << ", ";
            }
            querybuilder << "(" << player->getID() << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player scores %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    
    PlayerView* playerview = player->getPlayerView();
    std::set<uint32_t> idset = playerview->getUsableDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player usable designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = playerview->getUsableComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player usable components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = playerview->getOwnedObjects();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerobjectowned VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player owned objects %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

bool MysqlPersistence::updatePlayer(Player* player){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE player SET name='" << addslashes(player->getName()) << "', password='" << addslashes(player->getPass());
    querybuilder << "', email='" << addslashes(player->getEmail()) << "', comment='" << addslashes(player->getComment());
    querybuilder << "', boardid=" << player->getBoardId() << ", alive=" << ((player->isAlive()) ? 1 : 0);
    querybuilder << ", modtime=" << player->getModTime() << " WHERE playerid=" << player->getID() << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update player %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playerscore WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player score %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignusable WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player usable designs %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentusable WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player usable components %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM playerobjectowned WHERE playerid=" << player->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove player owned objects %d - %s", player->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    //save scores
    std::map<uint32_t, uint32_t> scores = player->getAllScores();
    if(!scores.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerscore VALUES ";
        for(std::map<uint32_t, uint32_t>::iterator itcurr = scores.begin(); itcurr != scores.end(); ++itcurr){
            if(itcurr != scores.begin()){
                querybuilder << ", ";
            }
            querybuilder << "(" << player->getID() << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store player scores %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    
    PlayerView* playerview = player->getPlayerView();
    std::set<uint32_t> idset = playerview->getUsableDesigns();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerdesignusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player usable designs %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }

    idset = playerview->getUsableComponents();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playercomponentusable VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player usable components %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    idset = playerview->getOwnedObjects();
    if(!idset.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO playerobjectowned VALUES ";
        for(std::set<uint32_t>::iterator itcurr = idset.begin(); itcurr != idset.end(); ++itcurr){
            if(itcurr != idset.begin())
                querybuilder << ", ";
            querybuilder << "(" << player->getID() << ", " << (*itcurr) << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update player owned objects %d - %s", player->getID(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

Player* MysqlPersistence::retrievePlayer(uint32_t playerid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM player WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player %d - %s", playerid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve player: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such player %d", playerid);
        mysql_free_result(obresult);
        return NULL;
    }
    Player* player = new Player();
    player->setId(playerid);
    player->setName(row[1]);
    player->setPass(row[2]);
    player->setEmail(row[3]);
    player->setComment(row[4]);
    player->setBoardId(atoi(row[5]));
    player->setIsAlive(atoi(row[6]) == 1);
    uint64_t modtime = strtoull(row[7], NULL, 10);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT * FROM playerscore WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player score %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player score: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    
    while((row = mysql_fetch_row(res)) != NULL){
      player->setScore(atoi(row[1]), atoi(row[2]));
    }
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignview WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible designs %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible designs: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    PlayerView* playerview = player->getPlayerView();
    
    std::set<uint32_t> idlist;
    
    while((row = mysql_fetch_row(res)) != NULL){
      idlist.insert(atoi(row[0]));
    }
    playerview->setVisibleDesigns(idlist);
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT designid FROM playerdesignusable WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player usable designs %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player usable designs: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    idlist.clear();
    while((row = mysql_fetch_row(res)) != NULL){
        idlist.insert(atoi(row[0]));
    }
    playerview->setUsableDesigns(idlist);
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentview WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible components %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible components: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    idlist.clear();
    while((row = mysql_fetch_row(res)) != NULL){
        idlist.insert(atoi(row[0]));
    }
    playerview->setVisibleComponents(idlist);
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT componentid FROM playercomponentusable WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player usable components %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player usable components: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    idlist.clear();
    while((row = mysql_fetch_row(res)) != NULL){
        idlist.insert(atoi(row[0]));
    }
    playerview->setUsableComponents(idlist);
    mysql_free_result(res);
    
    querybuilder.str("");
    querybuilder << "SELECT DISTINCT objectid FROM playerobjectview WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player visible objects %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player visible objects: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    idlist.clear();
    while((row = mysql_fetch_row(res)) != NULL){
        idlist.insert(atoi(row[0]));
    }
    mysql_free_result(res);
    playerview->setVisibleObjects(idlist);
    
    querybuilder.str("");
    querybuilder << "SELECT objectid FROM playerobjectowned WHERE playerid = " << playerid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve player owned objects %d - %s", playerid, mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    res = mysql_store_result(conn);
    if(res == NULL){
        Logger::getLogger()->error("Mysql: retrieve player owned objects: Could not store result - %s", mysql_error(conn));
        unlock();
        delete player;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    idlist.clear();
    while((row = mysql_fetch_row(res)) != NULL){
        idlist.insert(atoi(row[0]));
    }
    playerview->setOwnedObjects(idlist);
    mysql_free_result(res);
    
    player->setModTime(modtime);
    
    return player;
}

uint32_t MysqlPersistence::getMaxPlayerId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(playerid) FROM player;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max player id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max playerid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getPlayerIds(){
    lock();
    if(mysql_query(conn, "SELECT playerid FROM player;") != 0){
        Logger::getLogger()->error("Mysql: Could not query player ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get playerids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveCategory(Category* cat){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO category VALUES (" << cat->getCategoryId() << ", '" << cat->getName() << "', '";
    querybuilder << cat->getDescription() << "', " << cat->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store category %d - %s", cat->getCategoryId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}

Category* MysqlPersistence::retrieveCategory(uint32_t catid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM category WHERE categoryid = " << catid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve category %d - %s", catid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve category: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such category %d", catid);
        mysql_free_result(obresult);
        return NULL;
    }
    Category* cat = new Category();
    cat->setCategoryId(catid);
    cat->setName(row[1]);
    cat->setDescription(row[2]);
    cat->setModTime(strtoull(row[3], NULL, 10));
    mysql_free_result(obresult);
    return cat;
}

uint32_t MysqlPersistence::getMaxCategoryId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(categoryid) FROM category;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max category id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max categoryid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getCategoryIds(){
    lock();
    if(mysql_query(conn, "SELECT categoryid FROM category;") != 0){
        Logger::getLogger()->error("Mysql: Could not query category ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get categoryids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveDesign(Design* design){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO design VALUES (" << design->getDesignId() << ", " << design->getCategoryId() << ", '";
    querybuilder << addslashes(design->getName()) << "', '" << addslashes(design->getDescription()) << "', " << design->getOwner() << ", ";
    querybuilder << design->getInUse() << ", " << design->getNumExist() << ", " << design->isValid() << ", '";
    querybuilder << addslashes(design->getFeedback()) << "', " << design->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store design %d - %s", design->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    std::map<uint32_t, uint32_t> complist = design->getComponents();
    if(!complist.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO designcomponent VALUES ";
        for(std::map<uint32_t, uint32_t>::iterator itcurr = complist.begin(); itcurr != complist.end(); ++itcurr){
            if(itcurr != complist.begin())
                querybuilder << ", ";
            querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store design components %d - %s", design->getDesignId(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    std::map<uint32_t, PropertyValue> proplist = design->getPropertyValues();
    if(!proplist.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO designproperty VALUES ";
        for(std::map<uint32_t, PropertyValue>::iterator itcurr = proplist.begin(); itcurr != proplist.end(); ++itcurr){
            if(itcurr != proplist.begin())
                querybuilder << ", ";
            PropertyValue pv = itcurr->second;
            querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << pv.getValue() << ", '";
            querybuilder << addslashes(pv.getDisplayString()) << "')";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store design properties %d - %s", design->getDesignId(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

bool MysqlPersistence::updateDesign(Design* design){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE design SET categoryid=" << design->getCategoryId() << ", name='";
    querybuilder << addslashes(design->getName()) << "', description='" << addslashes(design->getDescription()) << "', owner=";
    querybuilder << design->getOwner() << ", inuse=" << design->getInUse() << ", numexist=" << design->getNumExist() << ", valid=";
    querybuilder << design->isValid() << ", feedback='" << addslashes(design->getFeedback());
    querybuilder << "', modtime=" << design->getModTime() << " WHERE designid=" << design->getDesignId() << ";";
    lock();
    std::string query = querybuilder.str();
    //std::cout << "Query: " << query << std::endl;
    if(mysql_query(conn, query.c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update design %d - %s", design->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM designcomponent WHERE designid=" << design->getDesignId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove design components %d - %s", design->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM designproperty WHERE designid=" << design->getDesignId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove design properties %d - %s", design->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    std::map<uint32_t, uint32_t> complist = design->getComponents();
    if(!complist.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO designcomponent VALUES ";
        for(std::map<uint32_t, uint32_t>::iterator itcurr = complist.begin(); itcurr != complist.end(); ++itcurr){
            if(itcurr != complist.begin())
                querybuilder << ", ";
            querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store design components %d - %s", design->getDesignId(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    std::map<uint32_t, PropertyValue> proplist = design->getPropertyValues();
    if(!proplist.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO designproperty VALUES ";
        for(std::map<uint32_t, PropertyValue>::iterator itcurr = proplist.begin(); itcurr != proplist.end(); ++itcurr){
            if(itcurr != proplist.begin())
                querybuilder << ", ";
            PropertyValue pv = itcurr->second;
            querybuilder << "(" << design->getDesignId() << ", " << itcurr->first << ", " << pv.getValue() << ", '";
            querybuilder << addslashes(pv.getDisplayString()) << "')";
        }
        querybuilder << ";";
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store design properties %d - %s", design->getDesignId(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

Design* MysqlPersistence::retrieveDesign(uint32_t designid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM design WHERE designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve design %d - %s", designid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve design: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such design %d", designid);
        mysql_free_result(obresult);
        return NULL;
    }
    Design* design = new Design();
    design->setDesignId(designid);
    design->setCategoryId(atoi(row[1]));
    design->setName(row[2]);
    design->setDescription(row[3]);
    design->setOwner(atoi(row[4]));
    design->setInUse(atoi(row[5]));
    design->setNumExist(atoi(row[6]));

        
    querybuilder.str("");
    querybuilder << "SELECT componentid,count FROM designcomponent WHERE designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve design components %d - %s", designid, mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    MYSQL_RES *compresult = mysql_store_result(conn);
    if(compresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve design components: Could not store result - %s", mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::map<uint32_t, uint32_t> complist;
    MYSQL_ROW crow;
    while((crow = mysql_fetch_row(compresult)) != NULL){
        complist[atoi(crow[0])] = atoi(crow[1]);
    }
    design->setComponents(complist);
    mysql_free_result(compresult);
    querybuilder.str("");
    querybuilder << "SELECT propertyid,value,displaystring FROM designproperty WHERE designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve design properties %d - %s", designid, mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    MYSQL_RES *propresult = mysql_store_result(conn);
    if(propresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve design properties: Could not store result - %s", mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::map<uint32_t, PropertyValue> pvlist;
    MYSQL_ROW prow;
    while((prow = mysql_fetch_row(propresult)) != NULL){
        PropertyValue pv;
        pv.setPropertyId(atoi(prow[0]));
        pv.setValue(atof(prow[1]));
        pv.setDisplayString(prow[2]);
        pvlist[pv.getPropertyId()] = pv;
    }
    design->setPropertyValues(pvlist);
    mysql_free_result(propresult);
    
    design->setValid(atoi(row[7]), row[8]);
    design->setModTime(strtoull(row[9], NULL, 10));
    
    mysql_free_result(obresult);
    
    return design;

}

uint32_t MysqlPersistence::getMaxDesignId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(designid) FROM design;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max design id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max designid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getDesignIds(){
    lock();
    if(mysql_query(conn, "SELECT designid FROM design;") != 0){
        Logger::getLogger()->error("Mysql: Could not query design ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get designids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveComponent(Component* comp){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO component VALUES (" << comp->getComponentId();
    querybuilder<< ", '" << addslashes(comp->getName()) << "', '" << addslashes(comp->getDescription()) << "', '";
    querybuilder << addslashes(comp->getTpclRequirementsFunction()) << "', " << comp->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store component %d - %s", comp->getComponentId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    std::set<uint32_t> catlist = comp->getCategoryIds();
    if(!catlist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO componentcat VALUES ";
      for(std::set<uint32_t>::iterator itcurr = catlist.begin(); itcurr != catlist.end();
          ++itcurr){
        if(itcurr != catlist.begin())
          querybuilder << ", ";
        querybuilder << "(" << comp->getComponentId() << ", " << (*itcurr) << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store component catergories %d - %s", comp->getComponentId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
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
        lock();
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store component properties %d - %s", comp->getComponentId(), mysql_error(conn));
            unlock();
            return false;
        }
        unlock();
    }
    return true;
}

Component* MysqlPersistence::retrieveComponent(uint32_t compid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM component WHERE componentid = " << compid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve component %d - %s", compid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve component: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such component %d", compid);
        mysql_free_result(obresult);
        return NULL;
    }
    Component* comp = new Component();
    comp->setComponentId(compid);
    comp->setName(row[1]);
    comp->setDescription(row[2]);
    comp->setTpclRequirementsFunction(row[3]);
    uint64_t modtime = strtoull(row[4], NULL, 10);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM componentcat WHERE componentid = " << compid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve component categories %d - %s", compid, mysql_error(conn));
      unlock();
      delete comp;
      return NULL;
    }
    MYSQL_RES *catresult = mysql_store_result(conn);
    if(catresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve component categories: Could not store result - %s", mysql_error(conn));
      unlock();
      delete comp;
      return NULL;
    }
    unlock();
    std::set<uint32_t> catids;
    while((row = mysql_fetch_row(catresult)) != NULL){
      catids.insert(atoi(row[0]));
    }
    comp->setCategoryIds(catids);
    mysql_free_result(catresult);
    
    querybuilder.str("");
    querybuilder << "SELECT propertyid,tpclvaluefunc FROM componentproperty WHERE componentid = " << compid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve component properties %d - %s", compid, mysql_error(conn));
        unlock();
        delete comp;
        return NULL;
    }
    MYSQL_RES *propresult = mysql_store_result(conn);
    if(propresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve component properties: Could not store result - %s", mysql_error(conn));
        unlock();
        delete comp;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::map<uint32_t, std::string> pvlist;
    while((row = mysql_fetch_row(propresult)) != NULL){
        pvlist[atoi(row[0])] = row[1];
    }
    comp->setPropertyList(pvlist);
    mysql_free_result(propresult);
    
    comp->setModTime(modtime);
    
    return comp;
}

uint32_t MysqlPersistence::getMaxComponentId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(componentid) FROM component;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max component id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max componentid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getComponentIds(){
    lock();
    if(mysql_query(conn, "SELECT componentid FROM component;") != 0){
        Logger::getLogger()->error("Mysql: Could not query component ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get componentids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveProperty(Property* prop){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO property VALUES (" << prop->getPropertyId() << ", ";
    querybuilder << prop->getRank() << ", '" << addslashes(prop->getName()) << "', '" << addslashes(prop->getDisplayName());
    querybuilder << "', '" << addslashes(prop->getDescription()) << "', '" << addslashes(prop->getTpclDisplayFunction()) << "', '";
    querybuilder << addslashes(prop->getTpclRequirementsFunction()) << "', " << prop->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store Property %d - %s", prop->getPropertyId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    std::set<uint32_t> catlist = prop->getCategoryIds();
    if(!catlist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO propertycat VALUES ";
      for(std::set<uint32_t>::iterator itcurr = catlist.begin(); itcurr != catlist.end();
          ++itcurr){
        if(itcurr != catlist.begin())
          querybuilder << ", ";
        querybuilder << "(" << prop->getPropertyId() << ", " << (*itcurr) << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store property catergories %d - %s", prop->getPropertyId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
    return true;
}

Property* MysqlPersistence::retrieveProperty(uint32_t propid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM property WHERE propertyid = " << propid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve property %d - %s", propid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve property: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such property %d", propid);
        mysql_free_result(obresult);
        return NULL;
    }
    Property* prop = new Property();
    prop->setPropertyId(propid);
    prop->setRank(atoi(row[1]));
    prop->setName(row[2]);
    prop->setDisplayName(row[3]);
    prop->setDescription(row[4]);
    prop->setTpclDisplayFunction(row[5]);
    prop->setTpclRequirementsFunction(row[6]);
    uint64_t modtime = strtoull(row[7], NULL, 10);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM propertycat WHERE propertyid = " << propid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve property categories %d - %s", propid, mysql_error(conn));
      unlock();
      delete prop;
      return NULL;
    }
    MYSQL_RES *catresult = mysql_store_result(conn);
    if(catresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve property categories: Could not store result - %s", mysql_error(conn));
      unlock();
      delete prop;
      return NULL;
    }
    unlock();
    std::set<uint32_t> catids;
    while((row = mysql_fetch_row(catresult)) != NULL){
      catids.insert(atoi(row[0]));
    }
    prop->setCategoryIds(catids);
    mysql_free_result(catresult);
    
    prop->setModTime(modtime);
    
    return prop;
}

uint32_t MysqlPersistence::getMaxPropertyId(){
    lock();
    if(mysql_query(conn, "SELECT MAX(propertyid) FROM property;") != 0){
        Logger::getLogger()->error("Mysql: Could not query max property id - %s", mysql_error(conn));
        unlock();
        return 0;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get max propertyid: Could not store result - %s", mysql_error(conn));
        return 0;
    }
    MYSQL_ROW max = mysql_fetch_row(obresult);
    uint32_t maxid = 0;
    if(max[0] != NULL){
        maxid = atoi(max[0]);
    }
    mysql_free_result(obresult);
    return maxid;
}

std::set<uint32_t> MysqlPersistence::getPropertyIds(){
    lock();
    if(mysql_query(conn, "SELECT propertyid FROM property;") != 0){
        Logger::getLogger()->error("Mysql: Could not query property ids - %s", mysql_error(conn));
        unlock();
        return std::set<uint32_t>();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    unlock();
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: get propertyids: Could not store result - %s", mysql_error(conn));
        return std::set<uint32_t>();
    }
    MYSQL_ROW max;
    std::set<uint32_t> vis;
    while((max = mysql_fetch_row(obresult)) != NULL){
        vis.insert(atoi(max[0]));
    }
    mysql_free_result(obresult);
    return vis;
}

bool MysqlPersistence::saveObjectView(uint32_t playerid, ObjectView*){
  //TODO
}

ObjectView* MysqlPersistence::retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn){
  //TODO
}

bool MysqlPersistence::saveDesignView(uint32_t playerid, DesignView*){
  //TODO
}

DesignView* MysqlPersistence::retrieveDesignView(uint32_t playerid, uint32_t designid){
  //TODO
}

bool MysqlPersistence::saveComponentView(uint32_t playerid, ComponentView*){
  //TODO
}

ComponentView* MysqlPersistence::retrieveComponentView(uint32_t playerid, uint32_t componentid){
  //TODO
}

std::string MysqlPersistence::addslashes(const std::string& in) const{
    char* buf = new char[in.length() * 2 + 1];
    uint len = mysql_real_escape_string(conn, buf, in.c_str(), in.length());
    std::string rtv(buf, len);
    delete[] buf;
    return rtv;
}

uint32_t MysqlPersistence::getTableVersion(const std::string& name){
    if(mysql_query(conn, (std::string("SELECT version FROM tableversion WHERE name='") + addslashes(name) + "';").c_str()) != 0){
        Logger::getLogger()->error("Mysql: table version query error: %s", mysql_error(conn));
        throw std::exception();
    }else{
        MYSQL_RES *tableversion = mysql_store_result(conn);
        if(tableversion == NULL){
            Logger::getLogger()->error("Mysql: table versions query result error: %s", mysql_error(conn));
            throw std::exception();
        }

        MYSQL_ROW row = mysql_fetch_row(tableversion);
        if(row == NULL || row[0] == NULL){ 
            Logger::getLogger()->warning("Mysql: table version not found");
            throw std::exception();
        }
        uint32_t version = atoi(row[0]);
        mysql_free_result(tableversion);
        return version;
    }
}



void MysqlPersistence::lock(){
}

void MysqlPersistence::unlock(){
}
