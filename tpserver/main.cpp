/*  Main method for tpserver-cpp
 *
 *  Copyright (C) 2003-2006  Lee Begg and the Thousand Parsec Project
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
#include <stdexcept>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "0.0.0"
#endif

#include "commandmanager.h"
#include "logging.h"
#include "game.h"
#include "net.h"
#include "settings.h"
#include "pluginmanager.h"

static void sigIntHandler(int signum){
  Network::getNetwork()->stopMainLoop();
}

static void child_handler(int signum)
{
    switch(signum){
    case SIGALRM:
        exit(1);    // fail, 2 seconds elapsed
        break;
    case SIGUSR1:
        exit(0);    // ok, child acknowledged
        break;
    case SIGCHLD:
        exit(1);    // fail, child died
        break;
    }
}

static void daemonize()
{
    pid_t pid, sid, parent;

    // already a daemon?
    if(getppid() == 1)
        return;

	// set up signal mask
	sigrelse(SIGCHLD);
	sigrelse(SIGUSR1);
	sigrelse(SIGALRM);
	sigrelse(SIGINT);
	sigrelse(SIGTERM);

    // trap signals
    signal(SIGCHLD,child_handler);
    signal(SIGUSR1,child_handler);
    signal(SIGALRM,child_handler);

    // fork
    if((pid = fork()) < 0){
        throw std::runtime_error(strerror(errno));
    }
    else if(pid > 0){
        // wait 2 seconds for child to acknowledge
        // pasuse() should not return (see child_handler())
        alarm(2);
        pause();
        exit(1);
    }
    
    parent = getppid();

    // handle signals
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);
    signal(SIGCHLD,SIG_DFL);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // change filemode mask
    umask(0);

    // new sid
    if((sid = setsid()) < 0)
        throw std::runtime_error(strerror(errno));

    if(chdir("/") < 0)
        throw std::runtime_error(strerror(errno));

    // redirect std file descriptors
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);

    // acknowledge to parent
    kill(parent, SIGUSR1);
}

int main(int argc, char **argv)
{
  Settings *mySettings = Settings::getSettings();
  mySettings->readArgs(argc, argv);

  if(mySettings->get("NEVER_START") != "!"){
    if(!mySettings->readConfFile()){
      std::string savedloglevel = mySettings->get("log_level");
      std::string savedlogconsole = mySettings->get("log_console");
      mySettings->set("log_level", "1");
      mySettings->set("log_console", "yes");
      Logger::getLogger()->error("Could not read config file '%s'", mySettings->get("config_file").c_str());
      mySettings->set("log_level", savedloglevel);
      mySettings->set("log_console", savedloglevel);
    }

    if(mySettings->get("DEBUG") != "!"){
        daemonize();
    }else{
        signal(SIGINT, sigIntHandler);
        signal(SIGTERM, sigIntHandler);
        signal(SIGPIPE, SIG_IGN);
        mySettings->set("log_console", "yes");
    }

	Logger *myLogger = Logger::getLogger();

	myLogger->info("Tpserver-cpp " VERSION " starting");
	myLogger->info("This is GPL software, please see the COPYING file");

        try{

	Game *myGame = Game::getGame();

          PluginManager* myPlugins = PluginManager::getPluginManager();
          myPlugins->start();

          try{

            std::string tpschemename = mySettings->get("tpscheme");
            if(tpschemename == "auto" || tpschemename == ""){
              
              //Temp. should be able to do better than this.
              if(myPlugins->loadTpScheme("tpguile")){
                myLogger->info("Loaded TpScheme tpguile");
              }else{
                myLogger->warning("Did not load TpScheme \"tpguile\", trying tpmzscheme");
                if(myPlugins->loadTpScheme("tpmzscheme")){
                  myLogger->info("Loaded TpScheme tpmzscheme");
                }else{
                  myLogger->warning("Did not load TpScheme \"tpmzscheme\"");
                }
              }
            }else{
              myLogger->info("Loading TpScheme %s", tpschemename.c_str());
              if(myPlugins->loadTpScheme(tpschemename)){
                myLogger->info("Loaded TpScheme %s", tpschemename.c_str());
              }else{
                myLogger->warning("Did not load TpScheme \"%s\"", tpschemename.c_str());
              }
            }

            std::string persistencename = mySettings->get("persistence");
            if(persistencename != ""){
              myLogger->info("Loading persistence method %s", persistencename.c_str());
              if(myPlugins->loadPersistence(persistencename)){
                myLogger->info("Loaded persistence method %s", persistencename.c_str());
              }else{
                myLogger->warning("Did not load persistence method \"%s\"", persistencename.c_str());
              }
            }

            std::string rulesetname = mySettings->get("ruleset");
            if(rulesetname != ""){
              myLogger->info("Loading ruleset %s", rulesetname.c_str());
              if(myPlugins->loadRuleset(rulesetname)){
                myLogger->info("Loaded ruleset %s", rulesetname.c_str());
              }else{
                myLogger->warning("Did not load ruleset \"%s\"", rulesetname.c_str());
              }
            }


            if(mySettings->get("game_load") == "yes"){
              myGame->load();
            }

            if(mySettings->get("game_start") == "yes"){
              myGame->start();
            }

            Network *myNetwork = Network::getNetwork();

            if(mySettings->get("network_start") == "yes"){
              myNetwork->start();
            }

	    myNetwork->adminStart();

            myNetwork->masterLoop();

            if(myNetwork->isStarted()){
              myNetwork->stop();
            }

            if(myGame->isLoaded()){
              myGame->saveAndClose();
            }

	    myNetwork->adminStop();
            
            }catch(std::exception e){
                myLogger->error("Caught exception: %s", e.what());
            }

          myPlugins->stop();

        }catch(std::exception e){
            myLogger->error("Caught exception, exiting. Exception: %s", e.what());
        }

	myLogger->info("TP-server exiting");
	myLogger->flush();

  }

  return 0;
}
