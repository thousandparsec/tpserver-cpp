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

#include <time.h>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef VERSION
#define VERSION "0.0.0"
#endif
#endif

#include "command.h"
#include "logging.h"
#include "settings.h"
#include "game.h"
#include "ruleset.h"
#include "turntimer.h"
#include "net.h"
#include "pluginmanager.h"
#include "algorithms.h"
#include "objectmanager.h"
#include "playermanager.h"

#include "commandmanager.h"

class EndTurnCommand : public Command{
  public:
    EndTurnCommand() : Command(){
        name = "turn-end";
        help = "End the current turn now and start processing.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        msg << Game::getGame()->getTurnTimer()->secondsToEOT() << " of "
            << Game::getGame()->getTurnTimer()->getTurnLength() << " seconds remain until EOT.";
        of->packInt(0);
        of->packString(msg.str());
    }
};

class TurnResetCommand : public Command{
  public:
    TurnResetCommand() : Command(){
        name = "turn-reset";
        help = "Reset the timer for the next turn.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        Logger::getLogger()->info("EOT timer reset by administrator.");
        Game::getGame()->getTurnTimer()->resetTimer();
        msg << "Reset EOT timer, " << Game::getGame()->getTurnTimer()->secondsToEOT() << " seconds remain until EOT.";
        of->packInt(0);
        of->packString(msg.str());
    }
};

class TurnNumberCommand : public Command{
  public:
    TurnNumberCommand() : Command(){
        name = "turn-number";
        help = "Gets the current turn number.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        msg << "The current turn number is " << Game::getGame()->getTurnNumber() << ".";
        of->packInt(0);
        of->packString(msg.str());
    }
};

class NetworkStartCommand : public Command{
  public:
    NetworkStartCommand() : Command(){
        name = "network-start";
        help = "Starts the network listeners and accepts connections.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
        name = "set-setting";
        help = "Sets a setting.";
        addCommandParameter(new CommandParameter(cpT_String, "setting", "Setting to set."));
        addCommandParameter(new CommandParameter(cpT_String, "value", "Value to set setting to."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string setting = frame->unpackString();
        std::string value = frame->unpackString();
        Settings::getSettings()->set(setting, value);
        msg << "Setting value of \"" << setting << "\" to \"" << value << "\".";
        of->packInt(0);
        of->packString(msg.str());
        Logger::getLogger()->info(msg.str().c_str());
    }
};

class SettingsGetCommand : public Command{
  public:
    SettingsGetCommand() : Command(){
        name = "get-setting";
        help = "Gets a setting.";
        addCommandParameter(new CommandParameter(cpT_String, "setting", "Setting to get."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string setting = frame->unpackString();
        msg << "Setting \"" << setting << "\" is set to \"" << Settings::getSettings()->get(setting) << "\".";
        of->packInt(0);
        of->packString(msg.str());
    }
};

class ReconfigureCommand : public Command{
  public:
    ReconfigureCommand() : Command(){
      name = "reconfigure";
      help = "Re-reads the configuration file.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {

      if(Network::getNetwork()->isStarted()) {
        Network::getNetwork()->stop();
      }
      if(Game::getGame()->isLoaded()) {
        Game::getGame()->saveAndClose();
      }

      bool read_conf  = Settings::getSettings()->readConfFile();
      bool net_start  = Settings::getSettings()->get("network_start") == "yes";
      bool game_start = Settings::getSettings()->get("game_start") == "yes";
      
      if(Settings::getSettings()->get("game_load") == "yes")
        Game::getGame()->load();
      
      if( net_start )
        Network::getNetwork()->start();
      
      if( game_start )
        Game::getGame()->start();
      
      if( read_conf && (!net_start || Network::getNetwork()->isStarted()) && (!game_start || Game::getGame()->isStarted())){
        of->packInt(0);
        of->packString("Successfully re-read configuration file.");
        Logger::getLogger()->info("Reconfigured by administrator.");
      }else{
        of->packInt(1);
        of->packString("Could not re-read configuration file.");
        Logger::getLogger()->error("Failed reconfiguration attempt by administrator.");
      }
    }
};

class PluginLoadCommand : public Command{
  public:
    PluginLoadCommand() : Command(){
        name = "plugin-load";
        help = "Loads a plugin.";
        addCommandParameter(new CommandParameter(cpT_String, "plugin", "Plugin to load."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string plugin = frame->unpackString();
        if(PluginManager::getPluginManager()->load(plugin)){
            msg << "Plugin \"" << plugin << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Plugin \"" << plugin << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str());
    }
};

class PluginListCommand : public Command{
  public:
    PluginListCommand() : Command(){
        name = "plugin-list";
        help = "Lists the plugins that are loaded.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        msg << "These plugins are loaded: " << PluginManager::getPluginManager()->getLoadedLibraryNames();
        of->packInt(0);
        of->packString(msg.str());
    }
};

class RulesetSetCommand : public Command{
  public:
    RulesetSetCommand() : Command(){
        name = "set-ruleset";
        help = "Sets the ruleset to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "ruleset", "Ruleset to load."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string ruleset = frame->unpackString();
        if(PluginManager::getPluginManager()->loadRuleset(ruleset)){
            msg << "Ruleset \"" << ruleset << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Ruleset \"" << ruleset << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str());
    }
};

class RulesetGetCommand : public Command{
  public:
    RulesetGetCommand() : Command(){
        name = "get-ruleset";
        help = "Gets the ruleset being used by the server.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        msg << "The server is using ruleset \"" << Game::getGame()->getRuleset()->getName()
            << "\" (version " << Game::getGame()->getRuleset()->getVersion() << ").";
        of->packInt(0);
        of->packString(msg.str());
    }
};

class TpschemeSetCommand : public Command{
  public:
    TpschemeSetCommand() : Command(){
        name = "set-tpscheme";
        help = "Sets the TpScheme implementation to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "tpscheme", "TpScheme implementation to load."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string tpscheme = frame->unpackString();
        if(PluginManager::getPluginManager()->loadTpScheme(tpscheme)){
            msg << "TpScheme implementation \"" << tpscheme << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "TpScheme implementation \"" << tpscheme << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str());
    }
};

class PersistenceSetCommand : public Command{
  public:
    PersistenceSetCommand() : Command(){
        name = "set-persistence";
        help = "Sets the persistence method to be used by the server.";
        addCommandParameter(new CommandParameter(cpT_String, "persist", "Persistence method to load."));
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        std::ostringstream msg;
        std::string persist = frame->unpackString();
        if(PluginManager::getPluginManager()->loadPersistence(persist)){
            msg << "Persistence method \"" << persist << "\" was loaded.";
            of->packInt(0);
        }else{
            msg << "Persistence method \"" << persist << "\" was not loaded.";
            of->packInt(1);
        }
        of->packString(msg.str());
    }
};

class GameLoadCommand : public Command{
  public:
    GameLoadCommand() : Command(){
        name = "game-load";
        help = "Loads the initial data for the game.";
    }
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
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
    void action( InputFrame::Ptr frame, OutputFrame::Ptr of) {
        if(Game::getGame()->isStarted()){
            of->packInt(0);
            of->packString("The game is started.");
        }else{
            of->packInt(1);
            of->packString("The game is not started.");
        }
    }
};

class StatusCommand : public Command{
    public:
        StatusCommand() : Command(){
            name = "status";
            help = "Prints out the key info about the server and game.";
        }
        void action( InputFrame::Ptr frame, OutputFrame::Ptr of){
            std::ostringstream formater;
            formater << "Server: tpserver-cpp" << std::endl;
            formater << "Version: " VERSION << std::endl;
            formater << "Persistence available: " << ((Game::getGame()->getPersistence() != NULL) ? "yes" : "no") << std::endl;
            Ruleset* rules = Game::getGame()->getRuleset();
            if(rules != NULL){
                formater << "Ruleset name: " << rules->getName() << std::endl;
                formater << "Ruleset Version: " << rules->getVersion() << std::endl;
            }
            formater << "Game Loaded: " << ((Game::getGame()->isLoaded()) ? "yes" : "no") << std::endl;
            formater << "Game Started: " << ((Game::getGame()->isStarted()) ? "yes" : "no") << std::endl;
            if(Game::getGame()->isStarted()){
                formater << "Time to next turn: " << Game::getGame()->getTurnTimer()->secondsToEOT() << std::endl;
                formater << "Turn length: " << Game::getGame()->getTurnTimer()->getTurnLength() << " seconds" << std::endl;
                formater << "Turn number: " << Game::getGame()->getTurnNumber() << std::endl;
                formater << "Turn name: " << Game::getGame()->getTurnName() << std::endl;
                formater << "Players not finished turn: " << Game::getGame()->getTurnTimer()->getPlayers().size();
                formater << " of " << Game::getGame()->getPlayerManager()->getNumPlayers() << std::endl;
            }
            formater << "Network Started: " << ((Network::getNetwork()->isStarted()) ? "yes" : "no") << std::endl;
            
            of->packInt(0);
            of->packString(formater.str());
            
        }
};

class ServerQuitCommand : public Command{
    public:
        ServerQuitCommand() : Command(){
            name = "server-quit";
            help = "Make the server exit, use with caution.";
        }
        void action( InputFrame::Ptr f, OutputFrame::Ptr of){
            Network::getNetwork()->stopMainLoop();
            of->packInt(0);
            of->packString("Server shutting down");
        }
};

class NukeObjectCommand : public Command{
    public:
        NukeObjectCommand() : Command(){
            name = "nuke-object";
            help = "Delete an object from the universe.";
            addCommandParameter(new CommandParameter(cpT_Integer, "objectid", "The object to remote."));
        }
        void action( InputFrame::Ptr f, OutputFrame::Ptr of){
            objectid_t objid = f->unpackInt();
            ObjectManager::Ptr objman = Game::getGame()->getObjectManager();
            IGObject::Ptr obj = objman->getObject(objid);
            obj->removeFromParent();
            objman->scheduleRemoveObject(objid);
            objman->clearRemovedObjects();
            PlayerManager::Ptr pm = Game::getGame()->getPlayerManager();
            IdSet playerids = pm->getAllIds();
            for(IdSet::iterator itpl = playerids.begin(); itpl != playerids.end(); ++itpl){
                PlayerView::Ptr pv = pm->getPlayer(*itpl)->getPlayerView();
                pv->removeVisibleObject(objid);
            }
            of->packInt(0);
            of->packString("Object nuked.");
        }
};

CommandManager *CommandManager::myInstance = NULL;

/* getCommandManager
 * Returns this unique instance of CommandManager.
 */
CommandManager *CommandManager::getCommandManager()
{
    if(myInstance == NULL)
      myInstance = new CommandManager();

    return myInstance;
}

/* checkCommandType
 * Verifies that a command type is valid.
 * type - The command type to check.
 */
bool CommandManager::checkCommandType(uint32_t type)
{
    return (type >= 0 && type < nextType);
}

/* describeCommand
 * Describes a command (if a valid type is passed).
 * cmdtype - The command type to describe.
 * of - The output frame.
 */
void CommandManager::describeCommand(uint32_t cmdtype, OutputFrame::Ptr of) 
{
    if(commandStore.find(cmdtype) != commandStore.end()){
        //call the command's describe function
        commandStore[cmdtype]->describeCommand(of);
    }else{
        throw FrameException(fec_NonExistant, "Command type does not exist");
    }
}

/* addCommandType
 * Adds a new command type to the command store.
 * cmd - The command prototype.
 */
void CommandManager::addCommandType(Command* cmd){
    cmd->setType(nextType);
    cmd->setDescriptionModTime(time(NULL));
    commandStore[nextType++] = cmd;
    seqkey++;
}

/* doGetCommandTypes
 * Processes a Get Command Type IDs frame.
 * frame - The input frame.
 * of - The output frame.
 */
void CommandManager::doGetCommandTypes(InputFrame::Ptr frame, OutputFrame::Ptr of) {
    uint32_t lseqkey = frame->unpackInt();
    if(lseqkey == UINT32_NEG_ONE){
        lseqkey = seqkey;
    }

    uint32_t start = frame->unpackInt();
    uint32_t num = frame->unpackInt();
    uint64_t fromtime = UINT64_NEG_ONE;
    if(frame->getVersion() >= fv0_4){
        fromtime = frame->unpackInt64();
    }
  
    if(lseqkey != seqkey){
        throw FrameException(fec_TempUnavailable, "Invalid Sequence Key");
    }

    IdModList modlist;
    for(std::map<uint32_t, Command*>::iterator itcurr = commandStore.begin(); itcurr != commandStore.end(); ++itcurr){
        Command* type = itcurr->second;
        if(fromtime == UINT64_NEG_ONE || type->getDescriptionModTime() > fromtime){
            modlist[itcurr->first] = type->getDescriptionModTime();
        }
    }

    if(start > modlist.size()){
        throw FrameException(fec_NonExistant, "Starting number too high");
    }
  
    if(num > modlist.size() - start){
        num = modlist.size() - start;
    }
  
    if(num > MAX_ID_LIST_SIZE + ((of->getVersion() < fv0_4) ? 1 : 0)){
        throw FrameException(fec_FrameError, "Too many items to get, frame too big");
    }

    of->setType(ftad_CommandTypes_List);
    of->packInt(lseqkey);
    of->packIdModList(modlist,num,start);
    if(of->getVersion() >= fv0_4){
        of->packInt64(fromtime);
    }
}

/* executeCommand
 * Performs the command action specified by a Command frame.
 * frame - The input frame.
 * of - The output frame.
 */
void CommandManager::executeCommand(InputFrame::Ptr frame, OutputFrame::Ptr of) 
{
    uint32_t cmdtype = frame->unpackInt();
    if(commandStore.find(cmdtype) != commandStore.end()){
        of->setType(ftad_CommandResult);
        //call the command's action function
        commandStore[cmdtype]->action(frame, of);
    }else{
        throw FrameException(fec_NonExistant, "Command type does not exist");
    }
}

/* Default constructor
 * Adds the local command set to the command store.
 */
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
    addCommandType(new ReconfigureCommand());
    addCommandType(new PluginLoadCommand());
    addCommandType(new PluginListCommand());
    addCommandType(new RulesetSetCommand());
    addCommandType(new RulesetGetCommand());
    addCommandType(new TpschemeSetCommand());
    addCommandType(new PersistenceSetCommand());
    addCommandType(new GameLoadCommand());
    addCommandType(new GameStartCommand());
    addCommandType(new GameIsStartedCommand());
    addCommandType(new StatusCommand());
    addCommandType(new ServerQuitCommand());
    addCommandType(new NukeObjectCommand());
}

/* Destructor
 * Clears out the command store.
 */
CommandManager::~CommandManager()
{
  delete_map_all( commandStore );
}
