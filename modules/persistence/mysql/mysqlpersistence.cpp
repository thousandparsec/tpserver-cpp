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
#include <tpserver/objecttypemanager.h>

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
            if(mysql_query(conn, "ALTER TABLE gameinfo APPEND turnname VARCHAR(50) NOT NULL;") != 0){
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
  std::ostringstream querybuilder;
  Game* game = Game::getGame();
  querybuilder << "INSERT INTO gameinfo VALUES ('" << addslashes(game->getKey()) << "', ";
  querybuilder << game->getGameStartTime() << ", " << game->getTurnNumber();
  querybuilder << ", '" << game->getTurnName() << "');";
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
  game->setTurnName(row[3]);
  
  mysql_free_result(giresult);
  
  return true;
}

bool MysqlPersistence::saveObject(IGObject* ob){
    std::ostringstream querybuilder;
    uint32_t turn = Game::getGame()->getTurnNumber();
    uint32_t obid = ob->getID();
    querybuilder << "DELETE FROM object WHERE objectid = " << obid << " AND turnnum = " << turn << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove old object %d - %s", obid, mysql_error(conn));
        unlock();
        return false;
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO object VALUES (" << obid << ", " << turn << ", ";
    querybuilder << (ob->isAlive() ? 1 : 0) << ", " << ob->getType() << ", ";
    querybuilder << "'" << addslashes(ob->getName()) << "', '" << addslashes(ob->getDescription()) << "', ";
    querybuilder << ob->getParent() << ", " << ob->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store object %d - %s", obid, mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    bool rtv = true;
    //store type-specific information
    if(ob->isAlive()){
      try{
        std::map<uint32_t, ObjectParameterGroupPtr> groups = ob->getParameterGroups();
        for(std::map<uint32_t, ObjectParameterGroupPtr>::iterator itcurr = groups.begin();
                itcurr != groups.end(); ++itcurr){
            ObjectParameterGroupData::ParameterList params = itcurr->second->getParameters();
            uint32_t ppos = 0;
            for(ObjectParameterGroupData::ParameterList::iterator paramcurr = params.begin();
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
                    throw new std::exception();
                    break;
                }
                ppos++;
            }
        }
        
        
      }catch(std::exception* e){
        rtv = false;
      }
    }
    ob->setIsDirty(!rtv);

    return rtv;
}

IGObject* MysqlPersistence::retrieveObject(uint32_t obid){
    std::ostringstream querybuilder;
    uint32_t turn = Game::getGame()->getTurnNumber();
    querybuilder << "SELECT * FROM object WHERE objectid = " << obid << " AND turnnum <= " << turn;
    querybuilder << " ORDER BY turnnum DESC LIMIT 1;";
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
    querybuilder << "SELECT object.objectid FROM object JOIN (SELECT objectid, MAX(turnnum) AS maxturnnum FROM object WHERE turnnum <= " << turn << " GROUP BY objectid) AS maxx ON (object.objectid=maxx.objectid AND object.turnnum = maxx.maxturnnum) WHERE parentid = " << obid << " ;";
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
    Game::getGame()->getObjectTypeManager()->setupObject(object, atoi(row[3]));
    object->setIsAlive(atoi(row[2]) == 1);
    object->setName(row[4]);
    object->setDescription(row[5]);
    object->setParent(atoi(row[6]));
    
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
                    throw new std::exception();
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
    
    object->setModTime(strtoull(row[7], NULL, 10));
    object->setIsDirty(false);
    
    mysql_free_result(obresult);
    return object;
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
  std::set<uint32_t> ordertypes = oq->getAllowedOrderTypes();
  if(!ordertypes.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderqueueallowedtype VALUES ";
    for(std::set<uint32_t>::iterator itcurr = ordertypes.begin(); itcurr != ordertypes.end(); ++itcurr){
      if(itcurr != ordertypes.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << (*itcurr) << ")";
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderqueue allowed types - %s", mysql_error(conn));
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
  querybuilder << ", repeating=" << (oq->isRepeating() ? 1 : 0) << ", modtime=" << oq->getModTime()<< " WHERE queueid=" << oq->getQueueId() << ";";
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
  querybuilder.str("");
  querybuilder << "DELETE FROM orderqueueallowedtype WHERE queueid=" << oq->getQueueId() <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
    Logger::getLogger()->error("Mysql: Could not remove allowed types for orderqueue %d - %s", oq->getQueueId(), mysql_error(conn));
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
  std::set<uint32_t> ordertypes = oq->getAllowedOrderTypes();
  if(!ordertypes.empty()){
    querybuilder.str("");
    querybuilder << "INSERT INTO orderqueueallowedtype VALUES ";
    for(std::set<uint32_t>::iterator itcurr = ordertypes.begin(); itcurr != ordertypes.end(); ++itcurr){
      if(itcurr != ordertypes.begin()){
        querybuilder << ", ";
      }
      querybuilder << "(" << oq->getQueueId() << ", " << (*itcurr) << ")";
    }
    querybuilder << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not store orderqueue allowed types - %s", mysql_error(conn));
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
  querybuilder.str("");
  querybuilder << "SELECT ordertype FROM orderqueueallowedtype WHERE queueid=" << oqid <<";";
  if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not query orderqueue allowed types - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      mysql_free_result(ordresult);
      mysql_free_result(ownerresult);
      return NULL;
  }
  MYSQL_RES *allowedresult = mysql_store_result(conn);
  if(allowedresult == NULL){
      Logger::getLogger()->error("Mysql: get order queue allowed types: Could not store result - %s", mysql_error(conn));
      unlock();
      mysql_free_result(oqresult);
      mysql_free_result(ordresult);
      mysql_free_result(ownerresult);
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
  
  MYSQL_ROW allowrow;
  std::set<uint32_t> allowedorders;
  while((allowrow = mysql_fetch_row(allowedresult)) != NULL){
    allowedorders.insert(atoi(allowrow[0]));
  }
  mysql_free_result(allowedresult);
  oq->setAllowedOrderTypes(allowedorders);
  
  oq->setModTime(strtoull(oqrow[4], NULL, 10));
  
  mysql_free_result(oqresult);
  
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
    order->setOrderQueueId(queueid);
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

bool MysqlPersistence::saveObjectView(uint32_t playerid, ObjectView* ov){
    std::ostringstream querybuilder;
    uint32_t turnnum =  Game::getGame()->getTurnNumber();
    querybuilder << "DELETE FROM playerobjectview WHERE playerid=" << playerid << " AND objectid=" << ov->getObjectId() << " AND turnnum = " << turnnum << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove objectview %d,%d - %s", playerid, ov->getObjectId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    unlock();
    
    querybuilder.str("");
    querybuilder << "INSERT INTO playerobjectview VALUES (" << playerid << ", " << ov->getObjectId() << ", ";
    querybuilder << turnnum << ", " << ((ov->isCompletelyVisible()) ? 1 : 0) << ", "; 
    querybuilder << ((ov->isGone()) ? 1 : 0) << ", " << ((ov->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(ov->getVisibleName()) << "', " << ((ov->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(ov->getVisibleDescription()) << "', " << ov->getModTime() << ");";
    
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store objectview %d,%d - %s", playerid, ov->getObjectId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    //params?
    
    return true;
}

ObjectView* MysqlPersistence::retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM playerobjectview WHERE playerid = " << playerid << " AND objectid = " << objectid << " AND turnnum <= " << turn << " ORDER BY turnnum DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve objectview %d,%d - %s", playerid, objectid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve objectview: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No suchobjectview %d,%d", playerid, objectid);
        mysql_free_result(obresult);
        return NULL;
    }
    ObjectView* obj = new ObjectView();
    obj->setObjectId(objectid);
    obj->setCompletelyVisible(atoi(row[3]) == 1);
    obj->setGone(atoi(row[4]) == 1);
    obj->setCanSeeName(atoi(row[5]) == 1);
    obj->setVisibleName(row[6]);
    obj->setCanSeeDescription(atoi(row[7]) == 1);
    obj->setVisibleDescription(row[8]);
    uint64_t modtime = strtoull(row[9], NULL, 10);
    mysql_free_result(obresult);
    
    //params?
    
    obj->setModTime(modtime);
    return obj;
}

bool MysqlPersistence::saveDesignView(uint32_t playerid, DesignView* dv){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM playerdesignview WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove designview %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignviewcomp WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove designview ccomponents %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playerdesignviewprop WHERE playerid=" << playerid << " AND designid=" << dv->getDesignId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove designview properties %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    unlock();
    
    querybuilder.str("");
    querybuilder << "INSERT INTO playerdesignview VALUES (" << playerid << ", " << dv->getDesignId() << ", ";
    querybuilder << ((dv->isCompletelyVisible()) ? 1 : 0) << ", " << ((dv->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(dv->getVisibleName()) << "', " << ((dv->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(dv->getVisibleDescription()) << "', " << ((dv->canSeeNumExist()) ? 1 : 0) << ", ";
    querybuilder << dv->getVisibleNumExist() << ", " << ((dv->canSeeOwner()) ? 1 : 0) << ", ";
    querybuilder << dv->getVisibleOwner() << ", " << dv->getModTime() << ");";
    
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store designview %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    std::map<uint32_t, uint32_t> complist = dv->getVisibleComponents();
    if(!complist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO playerdesignviewcomp VALUES ";
      for(std::map<uint32_t,uint32_t>::iterator itcurr = complist.begin(); itcurr != complist.end();
          ++itcurr){
        if(itcurr != complist.begin())
          querybuilder << ", ";
        querybuilder << "(" << playerid << ", " << dv->getDesignId() << ", " << (itcurr->first) << ", " << (itcurr->second) << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store designview catergories %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
    std::map<uint32_t, PropertyValue> proplist = dv->getVisiblePropertyValues();
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
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store designview properties %d,%d - %s", playerid, dv->getDesignId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
    return true;
}

DesignView* MysqlPersistence::retrieveDesignView(uint32_t playerid, uint32_t designid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM playerdesignview WHERE playerid = " << playerid << " AND designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve designview %d,%d - %s", playerid, designid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve designview: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such designview %d,%d", playerid, designid);
        mysql_free_result(obresult);
        return NULL;
    }
    DesignView* design = new DesignView();
    design->setDesignId(designid);
    design->setIsCompletelyVisible(atoi(row[2]) == 1);
    design->setCanSeeName(atoi(row[3]) == 1);
    design->setVisibleName(row[4]);
    design->setCanSeeDescription(atoi(row[5]) == 1);
    design->setVisibleDescription(row[6]);
    design->setCanSeeNumExist(atoi(row[7]) == 1);
    design->setVisibleNumExist(atoi(row[8]));
    design->setCanSeeOwner(atoi(row[9]) == 1);
    design->setVisibleOwner(atoi(row[10]));
    uint64_t modtime = strtoull(row[11], NULL, 10);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT componentid,quantity FROM playerdesignviewcomp WHERE playerid = " << playerid << " AND designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve designview components %d,%d - %s", playerid, designid, mysql_error(conn));
      unlock();
      delete design;
      return NULL;
    }
    MYSQL_RES *compresult = mysql_store_result(conn);
    if(compresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve designview components: Could not store result - %s", mysql_error(conn));
      unlock();
      delete design;
      return NULL;
    }
    unlock();
    std::map<uint32_t,uint32_t> comps;
    while((row = mysql_fetch_row(compresult)) != NULL){
      comps[atoi(row[0])] = atoi(row[1]);
    }
    design->setVisibleComponents(comps);
    mysql_free_result(compresult);
    
    querybuilder.str("");
    querybuilder << "SELECT propertyid,value FROM playerdesignviewprop WHERE playerid = " << playerid << " AND designid = " << designid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve designview properties %d,%d - %s", playerid, designid, mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    MYSQL_RES *propresult = mysql_store_result(conn);
    if(propresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve designview properties: Could not store result - %s", mysql_error(conn));
        unlock();
        delete design;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::map<uint32_t, PropertyValue> pvlist;
    while((row = mysql_fetch_row(propresult)) != NULL){
        PropertyValue pv;
        pv.setPropertyId(atoi(row[0]));
        pv.setDisplayString(row[1]);
        pvlist[pv.getPropertyId()] = pv;
    }
    design->setVisiblePropertyValues(pvlist);
    mysql_free_result(propresult);
    
    design->setModTime(modtime);
    
    return design;
}

bool MysqlPersistence::saveComponentView(uint32_t playerid, ComponentView* cv){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM playercomponentview WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove componentview %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentviewcat WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove componentview categories %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    querybuilder.str("");
    querybuilder << "DELETE FROM playercomponentviewproperty WHERE playerid=" << playerid << " AND componentid=" << cv->getComponentId() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove componentview properties %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
    }
    
    unlock();
    
    querybuilder.str("");
    querybuilder << "INSERT INTO playercomponentview VALUES (" << playerid << ", " << cv->getComponentId() << ", ";
    querybuilder << ((cv->isCompletelyVisible()) ? 1 : 0) << ", " << ((cv->canSeeName()) ? 1 : 0) << ", '";
    querybuilder << addslashes(cv->getVisibleName()) << "', " << ((cv->canSeeDescription()) ? 1 : 0) << ", '";
    querybuilder << addslashes(cv->getVisibleDescription()) << "', " << ((cv->canSeeRequirementsFunc()) ? 1 : 0) << ", ";
    querybuilder << cv->getModTime() << ");";
    
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store componentview %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    
    std::set<uint32_t> catlist = cv->getVisibleCategories();
    if(!catlist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO playercomponentviewcat VALUES ";
      for(std::set<uint32_t>::iterator itcurr = catlist.begin(); itcurr != catlist.end();
          ++itcurr){
        if(itcurr != catlist.begin())
          querybuilder << ", ";
        querybuilder << "(" << playerid << ", " << cv->getComponentId() << ", " << (*itcurr) << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store componentview catergories %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
    std::set<uint32_t> proplist = cv->getVisiblePropertyFuncs();
    if(!proplist.empty()){
      querybuilder.str("");
      querybuilder << "INSERT INTO playercomponentviewproperty VALUES ";
      for(std::set<uint32_t>::iterator itcurr = proplist.begin(); itcurr != proplist.end();
          ++itcurr){
        if(itcurr != proplist.begin())
          querybuilder << ", ";
        querybuilder << "(" << playerid << ", " << cv->getComponentId() << ", " << (*itcurr) << ")";
      }
      querybuilder << ";";
      lock();
      if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store componentview properties %d,%d - %s", playerid, cv->getComponentId(), mysql_error(conn));
        unlock();
        return false;
      }
      unlock();
    }
    
    return true;
}

ComponentView* MysqlPersistence::retrieveComponentView(uint32_t playerid, uint32_t componentid){
    std::ostringstream querybuilder;
    querybuilder << "SELECT * FROM playercomponentview WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve componentview %d,%d - %s", playerid, componentid, mysql_error(conn));
        unlock();
        return NULL;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve componentview: Could not store result - %s", mysql_error(conn));
        unlock();
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such componentview %d,%d", playerid, componentid);
        mysql_free_result(obresult);
        return NULL;
    }
    ComponentView* comp = new ComponentView();
    comp->setComponentId(componentid);
    comp->setCompletelyVisible(atoi(row[2]) == 1);
    comp->setCanSeeName(atoi(row[3]) == 1);
    comp->setVisibleName(row[4]);
    comp->setCanSeeDescription(atoi(row[5]) == 1);
    comp->setVisibleDescription(row[6]);
    comp->setCanSeeRequirementsFunc(atoi(row[7]) == 1);
    uint64_t modtime = strtoull(row[8], NULL, 10);
    mysql_free_result(obresult);
    
    querybuilder.str("");
    querybuilder << "SELECT categoryid FROM playercomponentviewcat WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
      Logger::getLogger()->error("Mysql: Could not retrieve componentview categories %d,%d - %s", playerid, componentid, mysql_error(conn));
      unlock();
      delete comp;
      return NULL;
    }
    MYSQL_RES *catresult = mysql_store_result(conn);
    if(catresult == NULL){
      Logger::getLogger()->error("Mysql: retrieve componentview categories: Could not store result - %s", mysql_error(conn));
      unlock();
      delete comp;
      return NULL;
    }
    unlock();
    std::set<uint32_t> catids;
    while((row = mysql_fetch_row(catresult)) != NULL){
      catids.insert(atoi(row[0]));
    }
    comp->setVisibleCategories(catids);
    mysql_free_result(catresult);
    
    querybuilder.str("");
    querybuilder << "SELECT propertyid FROM playercomponentviewproperty WHERE playerid = " << playerid << " AND componentid = " << componentid << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve componentview properties %d,%d - %s", playerid, componentid, mysql_error(conn));
        unlock();
        delete comp;
        return NULL;
    }
    MYSQL_RES *propresult = mysql_store_result(conn);
    if(propresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve componentview properties: Could not store result - %s", mysql_error(conn));
        unlock();
        delete comp;
        return NULL;
    }
    unlock(); // finished with mysql for a bit
    
    std::set<uint32_t> pvlist;
    while((row = mysql_fetch_row(propresult)) != NULL){
        pvlist.insert(atoi(row[0]));
    }
    comp->setVisiblePropertyFuncs(pvlist);
    mysql_free_result(propresult);
    
    comp->setModTime(modtime);
    
    return comp;
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

bool MysqlPersistence::updatePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamposition WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old position3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparamposition VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << pob->getPosition().getX() << ", " << pob->getPosition().getY() << ", " << pob->getPosition().getZ() << ", " << pob->getRelative() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert position3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrievePosition3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Position3dObjectParam* pob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT posx,posy,posz,relative FROM objectparamposition WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve position3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve position3d param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such position3d param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    pob->setPosition(Vector3d(strtoll(row[0], NULL, 10), strtoll(row[1], NULL, 10), strtoll(row[2], NULL, 10)));
    pob->setRelative(atoi(row[3]));
    mysql_free_result(obresult);
    return true;
}

bool MysqlPersistence::updateVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob){
  std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamvelocity WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old velocity3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparamvelocity VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << vob->getVelocity().getX() << ", " << vob->getVelocity().getY() << ", " << vob->getVelocity().getZ() << ", " << vob->getRelative() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert velocity3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveVelocity3dObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, Velocity3dObjectParam* vob){
  std::ostringstream querybuilder;
    querybuilder << "SELECT velx,vely,velz,relative FROM objectparamvelocity WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve velocity3d param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve velocity3d param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such velocity3d param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    vob->setVelocity(Vector3d(strtoll(row[0], NULL, 10), strtoll(row[1], NULL, 10), strtoll(row[2], NULL, 10)));
    vob->setRelative(atoi(row[3]));
    mysql_free_result(obresult);
    return true;
}

bool MysqlPersistence::updateOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamorderqueue WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old orderqueue param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparamorderqueue VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << oob->getQueueId() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert orderqueue param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveOrderQueueObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, OrderQueueObjectParam* oob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT queueid FROM objectparamorderqueue WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve orderqueue param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve orderqueue param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such orderqueue param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    oob->setQueueId(atoi(row[0]));
    mysql_free_result(obresult);
    return true;
}

bool MysqlPersistence::updateResourceListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ResourceListObjectParam* rob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamresourcelist WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old resourcelist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
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
    
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert resourcelist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
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
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve resourcelist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve resourcelist param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist;
    MYSQL_ROW row;
    while((row = mysql_fetch_row(obresult)) != NULL){
        uint32_t available = atoi(row[1]);
        uint32_t possible = atoi(row[2]);
        if(available != 0 || possible != 0){
          reslist[atoi(row[0])] = std::pair<uint32_t, uint32_t>(available, possible);
        }
    }
    
    rob->setResources(reslist);
    
    mysql_free_result(obresult);
    
    return true;
}

bool MysqlPersistence::updateReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamreference WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old reference param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparamreference VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << rob->getReferenceType() << ", " << rob->getReferencedId() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert reference param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveReferenceObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, ReferenceObjectParam* rob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT reftype, refval FROM objectparamreference WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve reference param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve reference param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such reference param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    rob->setReferenceType(atoi(row[0]));
    rob->setReferencedId(atoi(row[1]));
    mysql_free_result(obresult);
    return true;
}

bool MysqlPersistence::updateRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamrefquantitylist WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old refquantitylist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
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
    
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert refquantitylist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveRefQuantityListObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, RefQuantityListObjectParam* rob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT reftype, refid, quant FROM objectparamrefquantitylist WHERE objectid = " << objid << " AND turn = (SELECT MAX(turn) FROM objectparamrefquantitylist WHERE objectid = " << objid;
    querybuilder << " AND turn <= " << turn << " AND playerid = " << plid;
    querybuilder << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos;
    querybuilder << ") AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve refquantitylist param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve refquantitylist param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    std::map<std::pair<int32_t, uint32_t>, uint32_t> reflist;
    MYSQL_ROW row;
    while((row = mysql_fetch_row(obresult)) != NULL){
        int32_t reftype = atoi(row[0]);
        uint32_t refid = atoi(row[1]);
        uint32_t quant = atoi(row[2]);
        if(reftype != 0 && refid != 0 && quant != 0){
          reflist[std::pair<int32_t, uint32_t>(reftype, refid)] = quant;
        }
    }
    
    rob->setRefQuantityList(reflist);
    
    mysql_free_result(obresult);
    
    return true;
}

bool MysqlPersistence::updateIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparaminteger WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old integer param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparaminteger VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << iob->getValue() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert integer param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveIntegerObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, IntegerObjectParam* iob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT val FROM objectparaminteger WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve integer param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve integer param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such integer param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    iob->setValue(atoi(row[0]));
    mysql_free_result(obresult);
    return true;
}

bool MysqlPersistence::updateSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM objectparamsize WHERE objectid = " << objid << " AND turn = " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << ";";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not delete old size param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO objectparamsize VALUES(" << objid << ", " << turn << ", " << plid << ", " << pgroup << ", " << pgpos << ", " << sob->getSize() << ");";
    
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not insert size param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock();
    return true;
}

bool MysqlPersistence::retrieveSizeObjectParam(uint32_t objid, uint32_t turn, uint32_t plid, uint32_t pgroup, uint32_t pgpos, SizeObjectParam* sob){
  std::ostringstream querybuilder;
    querybuilder << "SELECT size FROM objectparamsize WHERE objectid = " << objid << " AND turn <= " << turn << " AND playerid = " << plid << " AND paramgroupid = " << pgroup << " AND paramgrouppos = " << pgpos << " ORDER BY turn DESC LIMIT 1;";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve size param %d,%d - %s", objid, pgroup, mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve size param: Could not store result - %s", mysql_error(conn));
        unlock();
        throw new std::exception();
    }
    unlock(); // finished with mysql
    
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such size param %d,%d", objid, pgroup);
        mysql_free_result(obresult);
        throw new std::exception();
    }
    sob->setSize(strtoll(row[0], NULL, 10));
    mysql_free_result(obresult);
    return true;
}

void MysqlPersistence::lock(){
}

void MysqlPersistence::unlock(){
}
