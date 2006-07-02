/*  MysqlOrderMergeFleet class
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

#include <modules/games/minisec/mergefleet.h>
#include <tpserver/logging.h>
#include <modules/persistence/mysql/mysqlpersistence.h>

#include "mysqlordermergefleet.h"

MysqlOrderMergeFleet::~MysqlOrderMergeFleet(){
}

bool MysqlOrderMergeFleet::save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
     std::ostringstream querybuilder;
    querybuilder << "INSERT INTO mergefleet VALUES (" << ordid << ", " << static_cast<MergeFleet*>(ord)->getFleetId() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store mergefleet - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlOrderMergeFleet::update(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    return true;
}

bool MysqlOrderMergeFleet::retrieve(MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "SELECT fleetid FROM mergefleet WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve mergefleet - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL){
        Logger::getLogger()->error("Mysql: retrieve mergefleet: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such mergefleet %d", ordid);
        mysql_free_result(result);
        return false;
    }
    static_cast<MergeFleet*>(ord)->setFleetId(atoi(row[0]));
    mysql_free_result(result);
    return true;
}

bool MysqlOrderMergeFleet::remove(MYSQL* conn, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM mergefleet WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove mergefleet - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlOrderMergeFleet::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("mergefleet");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE mergefleet (orderid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "fleetid INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create mergefleet table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'mergefleet', 0);") != 0){
            Logger::getLogger()->debug("Could not set mergefleet table version");
        }
    }
}
