/*  MysqlOrderBuild class
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

#include "build.h"
#include "logging.h"
#include "mysqlpersistence.h"

#include "mysqlorderbuild.h"

MysqlOrderBuild::~MysqlOrderBuild(){
}

bool MysqlOrderBuild::save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO build VALUES (" << ordid << ", " << static_cast<Build*>(ord)->getTimeToGo() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store build - %s", mysql_error(conn));
        return false;
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO buildship VALUES ";
    std::map<uint32_t, uint32_t> ships = static_cast<Build*>(ord)->getShips();
    for(std::map<uint32_t, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
        if(itcurr != ships.begin())
            querybuilder << ", ";
        querybuilder << "(" << ordid << ", " << itcurr->first << ", " << itcurr->second << ")";
    }
    querybuilder << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store buildships - %s", mysql_error(conn));
        return false;
    }
    
    return true;
}

bool MysqlOrderBuild::retrieve(MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    
    querybuilder << "SELECT designid,count FROM buildship WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve build ships - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *shipresult = mysql_store_result(conn);
    if(shipresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve build ships: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(shipresult);
    while(row != NULL){
        static_cast<Build*>(ord)->addShips(atoi(row[0]), atoi(row[1]));
    }
    mysql_free_result(shipresult);
    
    querybuilder.str("");
    querybuilder << "SELECT timetogo FROM build WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve build - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL){
        Logger::getLogger()->error("Mysql: retrieve build: Could not store result - %s", mysql_error(conn));
        return false;
    }
    row = mysql_fetch_row(result);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such build %d", ordid);
        mysql_free_result(result);
        return false;
    }
    static_cast<Build*>(ord)->setTimeToGo(atoi(row[0]));
    mysql_free_result(result);
    return true;
}

bool MysqlOrderBuild::remove(MYSQL* conn, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM build WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove build - %s", mysql_error(conn));
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM buildship WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove buildship - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlOrderBuild::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("build");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE build (orderid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "turnstogo INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create build table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'build', 0);") != 0){
            Logger::getLogger()->debug("Could not set nop table version");
        }
    }
    try{
        uint32_t ver = persistence->getTableVersion("buildship");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE buildship (orderid INT UNSIGNED NOT NULL, "
                "designid INT UNSIGNED NOT NULL, count INT UNSIGNED NOT NULL, PRIMARY KEY (orderid, designid));") != 0){
            Logger::getLogger()->debug("Could not send query to create buildship table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'buildship', 0);") != 0){
            Logger::getLogger()->debug("Could not set buildship table version");
        }
    }
}
