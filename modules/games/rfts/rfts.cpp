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

// check
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tpserver/player.h>
#include <tpserver/game.h>

#include "rfts.h"

// check
// initial plugin-type enterance
extern "C" {
  #define tp_init librfts_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setRuleset(new Rfts());
  }
}

Rfts::Rfts() {

}

Rfts::~Rfts() {

}

std::string Rfts::getName() {
  return "Reach for the Stars";
}

std::string Rfts::getVersion() {
  return "0.0";
}

void Rfts::initGame() {

}

void Rfts::createGame() {

}

void Rfts::startGame() {

}

bool Rfts::onAddPlayer(Player *player) {

	return true;
}
void Rfts::onPlayerAdded(Player *player) {

}


