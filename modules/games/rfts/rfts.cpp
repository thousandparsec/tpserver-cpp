/*  RFTS rulesset
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>

#include <tpserver/property.h>
#include <tpserver/component.h>
#include <tpserver/design.h>
#include <tpserver/category.h>

#include <tpserver/objectdatamanager.h>
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>


#include "emptyobject.h"
#include "planet.h"

#include "rfts.h"

// hacky define :p (TMP!)
#define DEBUG_FN_PRINT() (Logger::getLogger()->debug(__FUNCTION__))

extern "C" {
  #define tp_init librfts_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setRuleset(new RFTS_::Rfts());
  }
}

namespace RFTS_ {

Rfts::Rfts() {

}

Rfts::~Rfts() {

}

std::string Rfts::getName() {
  return "TP RFTS";
}

std::string Rfts::getVersion() {
  return "0.0";
}

void Rfts::initGame() {
   DEBUG_FN_PRINT();
   
   // opt. set custom turn process.

   setObjectTypes();

   setOrderTypes();

}

void Rfts::setObjectTypes() {

   DEBUG_FN_PRINT();

   ObjectDataManager* obdm = Game::getGame()->getObjectDataManager();
   EmptyObject *ss = new EmptyObject();

   ss->setTypeName("Star System"); 
   ss->setTypeDescription("A system of stars!");
   
   obdm->addNewObjectType(ss);
   obdm->addNewObjectType(new Planet);
   //obdm->addNewObjectType(new Fleet);
}

void Rfts::setOrderTypes() {
   OrderManager* orm = Game::getGame()->getOrderManager();

   // todo (make orders :p)
}

void Rfts::createGame() {
   DEBUG_FN_PRINT();
   
   Game *game = game->getGame();
   
   void createDesignCategories();

      
   
   // set properties in DesignStore
   // ^^ crazy Tpcl stuff ^^

   // set components (using properties) in DesignStore

   // set up universe (universe -> galaxy -> star sys -> planets)
}

void Rfts::createDesignCategories() {
   DesignStore* ds = Game::getGame()->getDesignStore();

   Category*    cat = new Category();
   cat->setName("Battle Ships");
   cat->setDescription("The Battle ship design & component category");
   ds->addCategory(cat);
   
   cat = new Category();
   cat->setName("Non-battle Ships");
   cat->setDescription("The Non-battle ship design & component category");
   ds->addCategory(cat);
  
   cat = new Category();
   cat->setName("PDB");
   cat->setDescription("The Planetary Defense Base design & component category");
}

void Rfts::startGame() {
	DEBUG_FN_PRINT();
}

bool Rfts::onAddPlayer(Player *player) {
   DEBUG_FN_PRINT();
   if(Game::getGame()->getPlayerManager()->getNumPlayers() < MAX_PLAYERS)
      return true;
   return false;
}
void Rfts::onPlayerAdded(Player *player) {
   DEBUG_FN_PRINT();

   // set designs (?)
   // ^^ apply to player
}

}
