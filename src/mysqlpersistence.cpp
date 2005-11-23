/*  Mysql persistence class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "logging.h"
#include "settings.h"

#include "object.h"
#include "order.h"
#include "board.h"
#include "message.h"
#include "player.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"

#include "mysqlpersistence.h"

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
    if(mysql_real_connect(conn, host, user, pass, db, port, sock, 0) == NULL){
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
            if(mysql_query(conn, "INSERT INTO tableversion VALUES (NULL, 'tableversion', 0), (NULL, 'object', 0), "
                    "(NULL, 'order', 0), (NULL, 'orderslot', 0), (NULL, 'board', 0), (NULL, 'message', 0), (NULL, 'messageslot', 0), (NULL, 'player', 0), "
                    "(NULL, 'category', 0), (NULL, 'design',0), (NULL, 'component', 0), (NULL, 'property', 0);") != 0){
                throw std::exception();
            }
            if(mysql_query(conn, "CREATE TABLE object (objectid INT UNSIGNED NOT NULL PRIMARY KEY, type INT UNSIGNED NOT NULL, " 
                    "name TEXT NOT NULL, parentid INT UNSIGNED NOT NULL, size BIGINT UNSIGNED NOT NULL, posx BIGINT NOT NULL, "
                    "posy BIGINT NOT NULL, posz BIGINT NOT NULL, velx BIGINT NOT NULL, vely BIGINT NOT NULL, velz BIGINT NOT NULL, "
                    "orders INT UNSIGNED NOT NULL, modtime BIGINT UNSIGNED NOT NULL);") != 0){
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
        // Since this is the first release, there are no tables to update.
        mysql_free_result(tableversions);
    }
    
    unlock();
    return true;
}

void MysqlPersistence::shutdown(){
    lock();
    // TEMP HACK
    mysql_query(conn, "DELETE FROM object;");
    // end TEMP HACK
    if(conn != NULL){
        mysql_close(conn);
        conn = NULL;
    }
    unlock();
}

bool MysqlPersistence::saveObject(IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO object VALUES (" << ob->getID() << ", " << ob->getType() << ", ";
    querybuilder << "'" << addslashes(ob->getName()) << "', " << ob->getParent() << ", ";
    querybuilder << ob->getSize() << ", " << ob->getPosition().getX() << ", " << ob->getPosition().getY() << ", ";
    querybuilder << ob->getPosition().getZ() << ", " << ob->getVelocity().getX() << ", ";
    querybuilder << ob->getVelocity().getY() << ", " << ob->getVelocity().getZ() << ", " << ob->getNumOrders(-1);
    querybuilder << ", " << ob->getModTime() << ");";
    lock();
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store object %d - %s", ob->getID(), mysql_error(conn));
        unlock();
        return false;
    }
    unlock();
    return true;
}


std::string MysqlPersistence::addslashes(const std::string& in) const{
    char* buf = new char[in.length() * 2 + 1];
    uint len = mysql_real_escape_string(conn, buf, in.c_str(), in.length());
    std::string rtv(buf, len);
    delete[] buf;
    return rtv;
}

void MysqlPersistence::lock(){
}

void MysqlPersistence::unlock(){
}
