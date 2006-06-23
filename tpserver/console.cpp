/*  Server terminal console for server
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include <iostream>
#include <string.h>
#include <stdio.h>

#include <tprl/rlcommand.h>
#include <tprl/commandcategory.h>
#include <tprl/commandalias.h>
#include <tprl/console.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "net.h"
#include "game.h"
#include "object.h"
#include "settings.h"

#include "tpserver/console.h"

class QuitCommand : public tprl::RLCommand{
    public:
      QuitCommand() : tprl::RLCommand(){
            name = "quit";
            help = "Quit and shutdown the server.";
        }
        void action(const std::string& cmdline){
            Network::getNetwork()->stopMainLoop();
        }
};

class EndTurnCommand : public tprl::RLCommand{
    public:
      EndTurnCommand() : tprl::RLCommand(){
            name = "end";
            help = "End the current turn now and start processing.";
        }
        void action(const std::string& cmdline){
            Logger::getLogger()->info("End Of Turn initiated from console");
            Game::getGame()->doEndOfTurn();
            Game::getGame()->resetEOTTimer();
        }
};

class TurnTimeCommand : public tprl::RLCommand{
    public:
      TurnTimeCommand() : tprl::RLCommand(){
            name = "time";
            help = "The amount of time until the next turn.";
        }
        void action(const std::string& cmdline){
            std::cout << Game::getGame()->secondsToEOT() << " seconds to EOT" << std::endl;
        }
};

class TurnResetCommand : public tprl::RLCommand{
    public:
      TurnResetCommand() : tprl::RLCommand(){
            name = "reset";
            help = "Reset the timer for the next turn.";
        }
        void action(const std::string& cmdline){
            Game::getGame()->resetEOTTimer();
            std::cout << "Reset EOT timer, " << Game::getGame()->secondsToEOT() 
                    << " seconds to EOT" << std::endl;
        }
};

class TurnLengthCommand : public tprl::RLCommand{
    public:
      TurnLengthCommand() : tprl::RLCommand(){
            name = "length";
            help = "Sets the turn length, but doesn't affect the current turn.";
        }
        void action(const std::string& cmdline){
            uint32_t tlen = atoi(cmdline.c_str());
            Game::getGame()->setTurnLength(tlen);
            std::cout << "Setting turn length to " << tlen << " seconds" << std::endl;
        }
};

class TurnNumberCommand : public tprl::RLCommand{
  public:
    TurnNumberCommand() : tprl::RLCommand(){
      name = "number";
      help = "Gets the current turn number.";
    }
    void action(const std::string& cmdline){
      std::cout << "The current turn number is " << Game::getGame()->getTurnNumber() << std::endl;
    }
};

class NetworkStartCommand : public tprl::RLCommand{
    public:
      NetworkStartCommand() : tprl::RLCommand(){
            name = "start";
            help = "Starts the network listeners and accepts connections.";
        }
        void action(const std::string& cmdline){
            Network::getNetwork()->start();
        }
};

class NetworkStopCommand : public tprl::RLCommand{
    public:
      NetworkStopCommand() : tprl::RLCommand(){
            name = "stop";
            help = "Stops the network listeners and drops all connections.";
        }
        void action(const std::string& cmdline){
            Network::getNetwork()->stop();
        }
};

class SettingsSetCommand : public tprl::RLCommand{
    public:
      SettingsSetCommand() : tprl::RLCommand(){
            name = "set";
            help = "Sets a setting.";
        }
        void action(const std::string& cmdline){
            size_t pos1 = cmdline.find(' ');
            Settings::getSettings()->set(cmdline.substr(0, pos1), cmdline.substr(pos1+1));
            std::cout << "Setting value of \"" << cmdline.substr(0, pos1) << "\" to \"" << cmdline.substr(pos1+1) << "\"." << std::endl;
        }
};

class SettingsGetCommand : public tprl::RLCommand{
    public:
      SettingsGetCommand() : tprl::RLCommand(){
            name = "get";
            help = "Gets a setting.";
        }
        void action(const std::string& cmdline){
            std::cout << "Setting \"" << cmdline << "\" is set to \"" << Settings::getSettings()->get(cmdline) << "\"." << std::endl;
        }
};

Console *Console::myInstance = NULL;

Console *Console::getConsole()
{
	if (myInstance == NULL)
		myInstance = new Console();
	return myInstance;
}

void Console::open(){
  Network::getNetwork()->addConnection(this);
  if(console == NULL)
    console = tprl::Console::getConsole();
  console->setUseHistory(true);
  console->setCommandSet(&commands);
  console->readLine_nb_start();
  Logger::getLogger()->info("Console ready");
}

void Console::process(){
  console->readLine_nb_inputReady();
}

void Console::close()
{
  Network::getNetwork()->removeConnection(this);
  console->readLine_nb_stop();
  Logger::getLogger()->info("Console closed");
}


Console::Console() : Connection()
{
	Logger::getLogger()->info("Console opened");
	status = 1;
	sockfd = 2;
        console = NULL;

    commands.clear();
    tprl::CommandCategory* cat = new tprl::CommandCategory("turn","Turn management functions");
    cat->add(new EndTurnCommand());
    cat->add(new TurnTimeCommand());
    cat->add(new TurnResetCommand());
    cat->add(new TurnLengthCommand());
    cat->add(new TurnNumberCommand());
    commands.insert(cat);
    
    cat = new tprl::CommandCategory("network", "Network management functions");
    cat->add(new NetworkStartCommand());
    cat->add(new NetworkStopCommand());
    commands.insert(cat);
    
    cat = new tprl::CommandCategory("settings", "Setting control functions");
    cat->add(new SettingsSetCommand());
    cat->add(new SettingsGetCommand());
    commands.insert(cat);
    
    tprl::RLCommand* quit = new QuitCommand();
    commands.insert(quit);
    tprl::CommandAlias* alias = new tprl::CommandAlias("exit");
    alias->setTarget(quit);
    commands.insert(alias);

}

Console::~Console()
{
  if(console != NULL)
    delete console;
}
