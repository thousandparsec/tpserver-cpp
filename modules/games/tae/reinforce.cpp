/*  Reinforce order used in combat by colonist fleets. It adds one to
 *  the owner's strength for that round of combat.
 *
 *  Copyright (C) 2008 Dustin White and the Thousand Parsec Project
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

#include <cassert>

#include <tpserver/frame.h>
#include <tpserver/orderparameters.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/logging.h>

#include "fleet.h"
#include "taeturn.h"

#include "reinforce.h"

using std::set;
using std::map;
using std::string;
using std::stringstream;

Reinforce::Reinforce() : Order() {
    name = "Reinforce";
    description = "Send reinforcements to a conflict";
}

Reinforce::~Reinforce() {

}

Order* Reinforce::clone() const {
    Reinforce* o = new Reinforce();
    o->type = type;
    return o;
}

void Reinforce::createFrame(Frame *f, int pos) {
    Order::createFrame(f, pos);
}

void Reinforce::inputFrame(Frame *f, uint32_t playerid) {
    return;
}

bool Reinforce::doOrder(IGObject::Ptr obj) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();

    //Check to make sure obj is the right type
    if(obj->getType() != obtm->getObjectTypeByName("Fleet")) {
        Logger::getLogger()->debug("Player somehow issued a reinforce order on from an object other than a fleet");
        return false;
    }

    Fleet* fleetData = (Fleet*)(obj->getObjectBehaviour());
    uint32_t player = fleetData->getOwner();
    TaeTurn* turn = (TaeTurn*) Game::getGame()->getTurnProcess();

    //increment reinforcements
    turn->addReinforcement(player);

    //Remove object
    obj->removeFromParent();
    obm->scheduleRemoveObject(obj->getID());

    return true;
}
