#ifndef TAE_TAERULESET_H
#define TAE_TAERULESET_H

/*  TaE rulesset class
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

#include <tpserver/ruleset.h>


namespace tae {

class taeRuleset : public Ruleset {
    public:
        taeRuleset();
        virtual ~taeRuleset();

        std::string getName();
        std::string getVersion();
        void initGame();
        void createGame();
        void startGame();
        bool onAddPlayer(Player* player);
        void onPlayerAdded(Player* player);

    private:
        void setupResources();
        void createProperties();
        void createComponents();

        IGObject* createEmptyFleet(Player* owner, IGObject* parent, std::string name);
    
        Design* createPassengerShip(Player *owner, int type);
        Design* createVIPTransport(Player *owner, int type);
        Design* createBomber(Player *owner);

};

}
#endif
