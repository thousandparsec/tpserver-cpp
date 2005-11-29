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
#include "mysqlobjecttype.h"

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
        if(tableversions == NULL){
            Logger::getLogger()->error("Mysql: table versions query result error: %s", mysql_error(conn));
            Logger::getLogger()->error("You may need to delete the tables and start again");
            mysql_close(conn);
            conn = NULL;
            unlock();
            return false;
        }
        // Since this is the first release, there are no tables to update.
        mysql_free_result(tableversions);
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
    bool rtv;
    //store type-specific information
    MysqlObjectType* obtype = objecttypes[ob->getType()];
    if(obtype != NULL){
        rtv = obtype->save(this, conn, ob);
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", ob->getType());
        rtv = false;
    }
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
    object->setParent(atoi(row[3]));
    object->setSize(strtoull(row[4], NULL, 10));
    Vector3d vec;
    vec.setAll(atoll(row[5]), atoll(row[6]), atoll(row[7]));
    object->setPosition(vec);
    vec.setAll(atoll(row[8]), atoll(row[9]), atoll(row[10]));
    object->setVelocity(vec);
    object->setNumOrders(atoi(row[11]));
    
    MYSQL_ROW children;
    while((children = mysql_fetch_row(childres)) != NULL){
        object->addContainedObject(atoi(row[0]));
    }
    mysql_free_result(childres);

    // fetch type-specific information
    MysqlObjectType* obtype = objecttypes[object->getType()];
    if(obtype != NULL){
        lock();
        bool sucessful = obtype->retrieve(conn, object);
        unlock();
        object->setModTime(strtoull(row[12], NULL, 10));
        if(!sucessful){
            Logger::getLogger()->error("Mysql: Could not retrieve object type specific data");
            delete object;
            object = NULL;
        }
    }else{
        Logger::getLogger()->error("Mysql: Object type %d not registered", object->getType());
        delete object;
        object = NULL;
    }
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
            Logger::getLogger()->error("Mysql: table versions row error: %s", mysql_error(conn));
            throw std::exception();
        }
        uint32_t version = atoi(row[0]);
        mysql_free_result(tableversion);
        return version;
    }
}

void MysqlPersistence::addObjectType(MysqlObjectType* ot){
    objecttypes[ot->getType()] = ot;
    lock();
    ot->initialise(this, conn);
    unlock();
}

void MysqlPersistence::lock(){
}

void MysqlPersistence::unlock(){
}
