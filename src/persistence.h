#ifndef PERSISTENCE_H
#define PERSISTENCE_H
/*  Persistence class
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

#include <string>

class IGObject;
class Order;
class Board;
class Message;
class Player;
class Category;
class Design;
class Component;
class Property;

class Persistence{
public:
    virtual ~Persistence();

    virtual bool init();
    virtual void shutdown();

    virtual bool saveObject(IGObject* ob);
    virtual IGObject* retrieveObject(uint32_t obid);

    virtual bool saveOrder(Order* ord);
    virtual Order* retrieveOrder(uint32_t orpid);
    virtual Order* retrieveOrder(uint32_t obid, uint32_t slot);

    virtual bool saveBoard(Board* board);
    virtual Board* retrieveBoard(uint32_t boardid);

    virtual bool saveMessage(Message* msg);
    virtual Message* retrieveMessage(uint32_t msgpid);
    virtual Message* retrieveMessage(uint32_t boardid, uint32_t slot);

    virtual bool savePlayer(Player* player);
    virtual Player* retrievePlayer(uint32_t playerid);
    virtual Player* retrievePlayer(const std::string& name);

    virtual bool saveCategory(Category* cat);
    virtual Category* retriveCategory(uint32_t catid);

    virtual bool saveDesign(Design* design);
    virtual Design* retrieveDesign(uint32_t designid);

    virtual bool saveComponent(Component* comp);
    virtual Component* retrieveComponent(uint32_t compid);

    virtual bool saveProperty(Property* prop);
    virtual Property* retrieveProperty(uint32_t propid);

};

#endif
