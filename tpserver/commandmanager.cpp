/*  CommandManager for managing commands
 *
 *  Copyright (C) 2008 Aaron Mavrinac and the Thousand Parsec Project
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

#include <sstream>

#include "command.h"
#include "logging.h"
#include "settings.h"
#include "game.h"
#include "turntimer.h"
#include "net.h"
#include "pluginmanager.h"
#include "frame.h"

#include "commandmanager.h"

class EndTurnCommand : public Command{
  public:
    EndTurnCommand() : Command(){
        name = "turn-end";
        help = "End the current turn now and start processing.";
    }
    void action(Frame * frame, Frame * of){
        Logger::getLogger()->info("End of turn initiated by administrator.");
        Game::getGame()->getTurnTimer()->manuallyRunEndOfTurn();
        of->packInt(0);
        of->packString("End of turn initiated.");
    }
};

class TurnTimeCommand : public Command{
  public:
    TurnTimeCommand() : Command(){
        name = "turn-time";
        help = "The amount of time until the next turn.";
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        msg << Game::getGame()->getTurnTimer()->secondsToEOT() << " of "
            << Game::getGame()->getTurnTimer()->getTurnLength() << " seconds remain until EOT.";
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class TurnResetCommand : public Command{
  public:
    TurnResetCommand() : Command(){
        name = "turn-reset";
        help = "Reset the timer for the next turn.";
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        Game::getGame()->getTurnTimer()->resetTimer();
        msg << "Reset EOT timer, " << Game::getGame()->getTurnTimer()->secondsToEOT() << " seconds remain until EOT.";
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class TurnNumberCommand : public Command{
  public:
    TurnNumberCommand() : Command(){
        name = "turn-number";
        help = "Gets the current turn number.";
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        msg << "The current turn number is " << Game::getGame()->getTurnNumber() << ".";
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class NetworkStartCommand : public Command{
  public:
    NetworkStartCommand() : Command(){
        name = "network-start";
        help = "Starts the network listeners and accepts connections.";
    }
    void action(Frame * frame, Frame * of){
        Network::getNetwork()->start();
        if(Network::getNetwork()->isStarted()){
            of->packInt(0);
            of->packString("Network started.");
        }else{
            of->packInt(1);
            of->packString("Starting network failed, see log.");
        }
    }
};

class NetworkStopCommand : public Command{
  public:
    NetworkStopCommand() : Command(){
        name = "network-stop";
        help = "Stops the network listeners and drops all connections.";
    }
    void action(Frame * frame, Frame * of){
        Network::getNetwork()->stop();
        of->packInt(0);
        of->packString("Network stopped.");
    }
};

class NetworkIsStartedCommand : public Command{
  public:
    NetworkIsStartedCommand() : Command(){
        name = "network-isstarted";
        help = "Queries whether the network is started.";
    }
    void action(Frame * frame, Frame * of){
        if(Network::getNetwork()->isStarted()){
            of->packInt(0);
            of->packString("The network is started.");
        }else{
            of->packInt(1);
            of->packString("The network is stopped.");
        }
    }
};

class SettingsSetCommand : public Command{
  public:
    SettingsSetCommand() : Command(){
        name = "set";
        help = "Sets a setting.";
        addCommandParameter(new CommandParameter(cpT_String, "setting", "Setting to set."));
        addCommandParameter(new CommandParameter(cpT_String, "value", "Value to set setting to."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string setting = frame->unpackStdString();
        std::string value = frame->unpackStdString();
        Settings::getSettings()->set(setting, value);
        msg << "Setting value of \"" << setting << "\" to \"" << value << "\".";
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class SettingsGetCommand : public Command{
  public:
    SettingsGetCommand() : Command(){
        name = "get";
        help = "Gets a setting.";
        addCommandParameter(new CommandParameter(cpT_String, "setting", "Setting to get."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string setting = frame->unpackStdString();
        msg << "Setting \"" << setting << "\" is set to \"" << Settings::getSettings()->get(setting) << "\".";
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class PluginLoadCommand : public Command{
  public:
    PluginLoadCommand() : Command(){
        name = "plugin-load";
        help = "Loads a plugin.";
        addCommandParameter(new CommandParameter(cpT_String, "plugin", "Plugin to load."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string plugin = frame->unpackStdString();
        if(PluginManager::getPluginManager()->load(plugin)){
            msg << "Plugin \"" << plugin << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Plugin \"" << plugin << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str().c_str());
    }
};

class PluginListCommand : public Command{
  public:
    PluginListCommand() : Command(){
        name = "plugin-list";
        help = "Lists the plugins that are loaded.";
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        msg << "These plugins are loaded: " << PluginManager::getPluginManager()->getLoadedLibraryNames();
        of->packInt(0);
        of->packString(msg.str().c_str());
    }
};

class RulesetCommand : public Command{
  public:
    RulesetCommand() : Command(){
        name = "ruleset";
        help = "Sets the ruleset to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "ruleset", "Ruleset to load."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string ruleset = frame->unpackStdString();
        if(PluginManager::getPluginManager()->loadRuleset(ruleset)){
            msg << "Ruleset \"" << ruleset << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Ruleset \"" << ruleset << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str().c_str());
    }
};

class TpschemeCommand : public Command{
  public:
    TpschemeCommand() : Command(){
        name = "tpscheme";
        help = "Sets the TpScheme implementation to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "tpscheme", "TpScheme implementation to load."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string tpscheme = frame->unpackStdString();
        if(PluginManager::getPluginManager()->loadTpScheme(tpscheme)){
            msg << "TpScheme implementation \"" << tpscheme << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "TpScheme implementation \"" << tpscheme << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str().c_str());
    }
};

class PersistenceCommand : public Command{
  public:
    PersistenceCommand() : Command(){
        name = "persistence";
        help = "Sets the persistence method to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "persist", "Persistence method to load."));
    }
    void action(Frame * frame, Frame * of){
        std::ostringstream msg;
        std::string persist = frame->unpackStdString();
        if(PluginManager::getPluginManager()->loadPersistence(persist)){
            msg << "Persistence method \"" << persist << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Persistence method \"" << persist << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str().c_str());
    }
};

class GameLoadCommand : public Command{
  public:
    GameLoadCommand() : Command(){
        name = "game-load";
        help = "Loads the initial data for the game.";
    }
    void action(Frame * frame, Frame * of){
        if(Game::getGame()->load()){
            of->packInt(0);
            of->packString("Game loaded.");
        }else{
            of->packInt(1);
            of->packString("Loading game failed, see log.");
        }
    }
};

class GameStartCommand : public Command{
  public:
    GameStartCommand() : Command(){
        name = "game-start";
        help = "Starts the game and the EOT timer.";
    }
    void action(Frame * frame, Frame * of){
        if(Game::getGame()->start()){
            of->packInt(0);
            of->packString("Game started.");
        }else{
            of->packInt(1);
            of->packString("Starting game failed, see log.");
        }
    }
};

class GameIsStartedCommand : public Command{
  public:
    GameIsStartedCommand() : Command(){
        name = "game-isstarted";
        help = "Queries whether the game is started.";
    }
    void action(Frame * frame, Frame * of){
        if(Game::getGame()->isStarted()){
            of->packInt(0);
            of->packString("The game is started.");
        }else{
            of->packInt(1);
            of->packString("The game is not started.");
        }
    }
};

CommandManager *CommandManager::myInstance = NULL;

CommandManager *CommandManager::getCommandManager()
{
    if(myInstance == NULL)
      myInstance = new CommandManager();

    return myInstance;
}

bool CommandManager::checkCommandType(uint32_t type)
{
    return (type >= 0 && type < nextType);
}

void CommandManager::describeCommand(uint32_t cmdtype, Frame * of)
{
    if(commandStore.find(cmdtype) != commandStore.end()){
        commandStore[cmdtype]->describeCommand(of);
    }else{
        of->createFailFrame(fec_NonExistant, "Command type does not exist");
    }
}

void CommandManager::addCommandType(Command* cmd){
    cmd->setType(nextType);
    commandStore[nextType++] = cmd;
    seqkey++;
}

void CommandManager::doGetCommandTypes(Frame* frame, Frame * of){
    uint32_t lseqkey = frame->unpackInt();
    if(lseqkey == UINT32_NEG_ONE){
        //start new seqkey
        lseqkey = seqkey;
    }

    uint32_t start = frame->unpackInt();
    uint32_t num = frame->unpackInt();
  
    if(lseqkey != seqkey){
        of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
        return;
    }

    if(start > commandStore.size()){
        of->createFailFrame(fec_NonExistant, "Starting number too high");
        return;
    }
  
    if(num > commandStore.size() - start){
        num = commandStore.size() - start;
    }
  
    if(num > MAX_ID_LIST_SIZE + ((of->getVersion() < fv0_4) ? 1 : 0)){
        of->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
        return;
    }

    of->setType(ftad_CommandTypes_List);
    of->packInt(lseqkey);
    of->packInt(commandStore.size() - start - num);
    of->packInt(num);
    std::map<uint32_t, Command*>::iterator itcurr = commandStore.begin();
    advance(itcurr, start);
    for(uint32_t i = 0; i < num; i++){
        of->packInt(itcurr->first);
        ++itcurr;
    }
}

void CommandManager::executeCommand(Frame * frame, Frame * of)
{
    uint32_t cmdtype = frame->unpackInt();
    if(commandStore.find(cmdtype) != commandStore.end()){
        of->setType(ftad_CommandResult);
        commandStore[cmdtype]->action(frame, of);
    }else{
        of->createFailFrame(fec_NonExistant, "Command type does not exist");
    }
}

CommandManager::CommandManager()
{
    nextType = 0;

    addCommandType(new EndTurnCommand());
    addCommandType(new TurnTimeCommand());
    addCommandType(new TurnResetCommand());
    addCommandType(new TurnNumberCommand());
    addCommandType(new NetworkStartCommand());
    addCommandType(new NetworkStopCommand());
    addCommandType(new NetworkIsStartedCommand());
    addCommandType(new SettingsSetCommand());
    addCommandType(new SettingsGetCommand());
    addCommandType(new PluginLoadCommand());
    addCommandType(new PluginListCommand());
    addCommandType(new RulesetCommand());
    addCommandType(new TpschemeCommand());
    addCommandType(new PersistenceCommand());
    addCommandType(new GameLoadCommand());
    addCommandType(new GameStartCommand());
    addCommandType(new GameIsStartedCommand());
}

CommandManager::~CommandManager()
{
    for(std::map<uint32_t, Command*>::iterator itcurr = commandStore.begin(); itcurr != commandStore.end(); ++itcurr){
        delete itcurr->second;
    }
}
