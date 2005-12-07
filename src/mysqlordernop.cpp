/*  MysqlOrderNop class
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

#include "nop.h"
#include "logging.h"
#include "mysqlpersistence.h"

#include "mysqlordernop.h"

MysqlOrderNop::~MysqlOrderNop(){
}

bool MysqlOrderNop::save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO nop VALUES (" << ordid << ", " << static_cast<Nop*>(ord)->getTime() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store nop - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlOrderNop::update(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "UPDATE nop SET waittime=" << static_cast<Nop*>(ord)->getTime() << " WHERE orderid=" << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update nop - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlOrderNop::retrieve(MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "SELECT waittime FROM nop WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve nop - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL){
        Logger::getLogger()->error("Mysql: retrieve nop: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such nop %d", ordid);
        mysql_free_result(result);
        return false;
    }
    static_cast<Nop*>(ord)->setTime(atoi(row[0]));
    mysql_free_result(result);
    return true;
}

bool MysqlOrderNop::remove(MYSQL* conn, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM nop WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove nop - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlOrderNop::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("nop");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE nop (orderid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "waittime INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create nop table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'nop', 0);") != 0){
            Logger::getLogger()->debug("Could not set nop table version");
        }
    }
}
