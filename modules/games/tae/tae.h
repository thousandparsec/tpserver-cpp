#ifndef TAE_TAERULESET_H
#define TAE_TAERULESET_H

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

};

}
#endif
