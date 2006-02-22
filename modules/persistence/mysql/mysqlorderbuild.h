#ifndef MYSQLORDERBUILD_H
#define MYSQLORDERBUILD_H
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

#include <stdint.h>

#include "mysqlordertype.h"

typedef struct st_mysql MYSQL;
class MysqlPersistence;
class Order;

class MysqlOrderBuild : public MysqlOrderType{
public:
    virtual ~MysqlOrderBuild();

    virtual bool save(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord);
    virtual bool update(MysqlPersistence* persistence, MYSQL* conn, uint32_t ordid, Order* ord);
    virtual bool retrieve(MYSQL* conn, uint32_t ordid, Order* ord);
    virtual bool remove(MYSQL* conn, uint32_t ordid);
    
    virtual void initialise(MysqlPersistence* persistence, MYSQL* conn);

};

#endif
