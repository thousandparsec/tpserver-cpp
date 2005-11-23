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
    if(conn != NULL){
        Logger::getLogger()->warning("Mysql Persistence already running");
        return false;
    }
    conn = mysql_init(NULL);
    if(conn == NULL){
        Logger::getLogger()->error("Could not init mysql");
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
        return false;
    }
    return true;
}

void MysqlPersistence::shutdown(){
    if(conn != NULL){
        mysql_close(conn);
        conn = NULL;
    }
}
