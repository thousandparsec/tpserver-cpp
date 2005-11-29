/*  MysqlUniverse class
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

#include "mysqlpersistence.h"
#include "logging.h"
#include "object.h"
#include "universe.h"

#include "mysqluniverse.h"

MysqlUniverse::~MysqlUniverse(){
}

bool MysqlUniverse::save(MysqlPersistence* persistence, MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO universe VALUES (" << ob->getID() << ", " << ((Universe*)(ob->getObjectData()))->getYear() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store universe - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlUniverse::retrieve(MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT year FROM universe WHERE objectid = " << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve universe - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve universe: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such universe %d", ob->getID());
        mysql_free_result(obresult);
        return false;
    }
    ((Universe*)(ob->getObjectData()))->setYear(atoi(row[0]));
    mysql_free_result(obresult);
    return true;
}

void MysqlUniverse::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("universe");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE universe (objectid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "year INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create universe table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'universe', 0);") != 0){
            Logger::getLogger()->debug("Could not set universe table version");
        }
    }
}
