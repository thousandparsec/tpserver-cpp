/*  MysqlOrderSplitFleet class
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

#include "splitfleet.h"
#include "logging.h"
#include "mysqlpersistence.h"

#include "mysqlordersplitfleet.h"

MysqlOrderSplitFleet::~MysqlOrderSplitFleet(){
}

bool MysqlOrderSplitFleet::save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    std::map<uint32_t, uint32_t> ships = static_cast<SplitFleet*>(ord)->getShips();
    if(!ships.empty()){
        std::ostringstream querybuilder;
        querybuilder << "INSERT INTO splitship VALUES ";
        std::map<uint32_t, uint32_t> ships = static_cast<SplitFleet*>(ord)->getShips();
        for(std::map<uint32_t, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
            if(itcurr != ships.begin())
                querybuilder << ", ";
            querybuilder << "(" << ordid << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not store splitships - %s", mysql_error(conn));
        return false;
        }
    }
    return true;
}

bool MysqlOrderSplitFleet::update(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord){
    return true;
}

bool MysqlOrderSplitFleet::retrieve(MYSQL* conn, uint32_t ordid, Order* ord){
    std::ostringstream querybuilder;
    
    querybuilder << "SELECT designid,count FROM splitship WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve splitfleet ships - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *shipresult = mysql_store_result(conn);
    if(shipresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve splitfleet ships: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row;
    while((row = mysql_fetch_row(shipresult)) != NULL){
        static_cast<SplitFleet*>(ord)->addShips(atoi(row[0]), atoi(row[1]));
    }
    mysql_free_result(shipresult);
    return true;
}

bool MysqlOrderSplitFleet::remove(MYSQL* conn, uint32_t ordid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM splitship WHERE orderid = " << ordid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove splitship - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlOrderSplitFleet::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("splitship");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE splitship (orderid INT UNSIGNED NOT NULL, "
                "designid INT UNSIGNED NOT NULL, count INT UNSIGNED NOT NULL, PRIMARY KEY (orderid, designid));") != 0){
            Logger::getLogger()->debug("Could not send query to create splitship table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'splitship', 0);") != 0){
            Logger::getLogger()->debug("Could not set splitship table version");
        }
    }
}
