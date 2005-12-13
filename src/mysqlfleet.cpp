/*  MysqlFleet class
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
#include "fleet.h"

#include "mysqlfleet.h"

MysqlFleet::~MysqlFleet(){
}

bool MysqlFleet::save(MysqlPersistence* persistence, MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    Fleet* fleet = (Fleet*)(ob->getObjectData());
    querybuilder << "INSERT INTO fleet VALUES (" << ob->getID() << ", " << fleet->getOwner() << ", ";
    querybuilder << fleet->getDamage() << "); ";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store fleet - %s", mysql_error(conn));
        return false;
    }
    querybuilder.str("");
    querybuilder << "INSERT INTO fleetship VALUES ";
    std::map<int,int> ships = fleet->getShips();
    for(std::map<int,int>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
        if(itcurr != ships.begin()){
            querybuilder << ", ";
        }
        querybuilder << "(" << ob->getID() << ", " << itcurr->first << ", " << itcurr->second << ")";
    }
    querybuilder << ";";
     if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not store fleet ships - %s", mysql_error(conn));
        return false;
    }
    return true;
}

bool MysqlFleet::update(MysqlPersistence* persistence, MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    Fleet* fleet = (Fleet*)(ob->getObjectData());
    querybuilder << "UPDATE fleet SET owner=" << fleet->getOwner() << ", damage=";
    querybuilder << fleet->getDamage() << " WHERE objectid=" << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update fleet - %s", mysql_error(conn));
        return false;
    }
    if(mysql_affected_rows(conn) != 1){
        Logger::getLogger()->error("Fleet doesn't exist in database, saving it");
        return save(persistence, conn, ob);
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM fleetship WHERE objectid=" << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not update clear fleetship - %s", mysql_error(conn));
        return false;
    }
    std::map<int,int> ships = fleet->getShips();
    if(!ships.empty()){
        querybuilder.str("");
        querybuilder << "INSERT INTO fleetship VALUES ";
        for(std::map<int,int>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
            if(itcurr != ships.begin()){
                querybuilder << ", ";
            }
            querybuilder << "(" << ob->getID() << ", " << itcurr->first << ", " << itcurr->second << ")";
        }
        querybuilder << ";";
        if(mysql_query(conn, querybuilder.str().c_str()) != 0){
            Logger::getLogger()->error("Mysql: Could not update fleet ships - %s", mysql_error(conn));
            return false;
        }
    }
    return true;
}

bool MysqlFleet::retrieve(MYSQL* conn, IGObject* ob){
    std::ostringstream querybuilder;
    querybuilder << "SELECT owner,damage FROM fleet WHERE objectid = " << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve fleet - %s", mysql_error(conn));
        return false;
    }
    MYSQL_RES *obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve fleet: Could not store result - %s", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(obresult);
    if(row == NULL){
        Logger::getLogger()->warning("Mysql: No such fleet %d", ob->getID());
        mysql_free_result(obresult);
        return false;
    }
    Fleet* fleet = (Fleet*)(ob->getObjectData());
    fleet->setOwner(atoi(row[0]));
    fleet->setDamage(atoi(row[1]));
    mysql_free_result(obresult);

    //fleet ships
    querybuilder.str("");
    querybuilder << "SELECT designid,number FROM fleetship WHERE objectid = " << ob->getID() << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not retrieve fleet ships - %s", mysql_error(conn));
        return false;
    }
    obresult = mysql_store_result(conn);
    if(obresult == NULL){
        Logger::getLogger()->error("Mysql: retrieve fleet ships: Could not store result - %s", mysql_error(conn));
        return false;
    }
    while((row= mysql_fetch_row(obresult)) != NULL){
        fleet->addShips(atoi(row[0]), atoi(row[1]));
    }
    mysql_free_result(obresult);
    return true;
}

bool MysqlFleet::remove(MYSQL* conn, uint32_t obid){
    std::ostringstream querybuilder;
    querybuilder << "DELETE FROM fleet WHERE objectid = " << obid << ";";
    if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove fleet - %s", mysql_error(conn));
        return false;
    }
    querybuilder.str("");
    querybuilder << "DELETE FROM fleetship WHERE objectid = " << obid << ";";
     if(mysql_query(conn, querybuilder.str().c_str()) != 0){
        Logger::getLogger()->error("Mysql: Could not remove fleetships - %s", mysql_error(conn));
        return false;
    }
    return true;
}

void MysqlFleet::initialise(MysqlPersistence* persistence, MYSQL* conn){
    try{
        uint32_t ver = persistence->getTableVersion("fleet");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE fleet (objectid INT UNSIGNED NOT NULL PRIMARY KEY, "
                "owner INT UNSIGNED NOT NULL, damage INT UNSIGNED NOT NULL);") != 0){
            Logger::getLogger()->debug("Could not send query to create fleet table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'fleet', 0);") != 0){
            Logger::getLogger()->debug("Could not set fleet table version");
        }
    }
    try{
        uint32_t ver = persistence->getTableVersion("fleetship");
        //initial version, no table version problems.
    }catch(std::exception){
        if(mysql_query(conn, "CREATE TABLE fleetship (objectid INT UNSIGNED NOT NULL, "
                "designid INT UNSIGNED NOT NULL, number INT UNSIGNED NOT NULL, PRIMARY KEY(objectid, designid));") != 0){
            Logger::getLogger()->debug("Could not send query to create fleetship table");
        }
        if(mysql_query(conn, "INSERT INTO tableversion VALUES(NULL, 'fleetship', 0);") != 0){
            Logger::getLogger()->debug("Could not set fleetship table version");
        }
    }
}
