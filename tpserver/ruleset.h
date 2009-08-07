#ifndef RULESET_H
#define RULESET_H
/*  Ruleset class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <boost/shared_ptr.hpp>

class Player;

class Ruleset{
  public:
        virtual ~Ruleset(){};
  virtual std::string getName() = 0;
  virtual std::string getVersion() = 0;
  virtual void initGame() = 0;
  virtual void createGame() = 0;
  virtual void startGame() = 0;
  virtual bool onAddPlayer(boost::shared_ptr<Player> player) = 0;
  virtual void onPlayerAdded(boost::shared_ptr<Player> player) = 0;
  
};

#endif
