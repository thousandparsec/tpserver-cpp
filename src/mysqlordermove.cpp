/*  MysqlOrderMove class
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

#include "move.h"
#include "logging.h"
#include "mysqlpersistence.h"

#include "mysqlordermove.h"

MysqlOrderMove::~MysqlOrderMove(){
}

bool MysqlOrderMove::save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    Vector3d dest = static_cast<Move*>(ord)->getDest();
    querybuilder << "INSERT INTO move VALUES (" << ordid << ", " << dest.getX() << ", "
            << dest.getY() << "," << dest.getZ() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store move - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlOrderMove::retrieve(MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    querybuilder << "SELECT destx,desty,destz FROM move WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve move - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if(result == NULL){
        Logger::getLogger()->error("Mysql: retrieve move: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such move %d", ordid);
        mysql_free_result(result);
        return false;
    }
    static_cast<Move*>(ord)->setDest(Vector3d(atol(row[0]), atol(row[1]), atol(row[2])));
    mysql_free_result(result);
    return true;
}

bool MysqlOrderMove::remove(MYSQL* conn, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM move WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove move - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlOrderMove::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("move");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE move (orderid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "destx BIGINT NOT NULL, desty BIGINT NOT NULL, destz BIGINT NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create move table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'move', 0);") != 0){
            Logger::getLogger()->debug("Could not set move table version");
        }
    }
}
