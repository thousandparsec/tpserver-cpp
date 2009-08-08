#ifndef MINISEC_H
#define MINISEC_H
/*  MiniSec rulesset class
 *
 *  Copyright (C) 2005, 2007, 2009  Lee Begg and the Thousand Parsec Project
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

#include <set>

#include <tpserver/ruleset.h>

class Random;
class Names{
    public:
        Names(const std::string& defaultname);
        virtual ~Names();
        
        virtual std::string getName();
        
    private:
        uint64_t systems;
        std::string prefix;
};

class MiniSec : public Ruleset {
public:

  MiniSec();
  virtual ~MiniSec();

  std::string getName();
  std::string getVersion();
  void initGame();
  void createGame();
  void startGame();
  bool onAddPlayer(boost::shared_ptr<Player> player);
  void onPlayerAdded(boost::shared_ptr<Player> player);
  
  Names* getFleetMediaNames() const;

private:
  IGObject::Ptr createStarSystem( IGObject::Ptr mw_galaxy, uint32_t& max_planets, Names* systemnames);

  Random* random;
  Names* systemmedia;
  Names* planetmedia;
  Names* fleetmedia;
};

#endif
