/*  MysqlPlanet class
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
#include "planet.h"

#include "mysqlplanet.h"

MysqlPlanet::~MysqlPlanet(){
}

bool MysqlPlanet::save(MysqlPersistence* persistence, MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "INSERT INTO planet VALUES (" << ob->getID() << ", " << ((Planet*)(ob->getObjectData()))->getOwner() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store planet - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlPlanet::retrieve(MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT owner FROM planet WHERE objectid = " << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve planet - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve planet: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such planet %d", ob->getID());
        mysql_free_result(obresult);
        return false;
    }
    ((Planet*)(ob->getObjectData()))->setOwner(atoi(row[0]));
    mysql_free_result(obresult);
    return true;
}

void MysqlPlanet::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("planet");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE planet (objectid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "owner INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create planet table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'planet', 0);") != 0){
            Logger::getLogger()->debug("Could not set planet table version");
        }
    }
}
