/*  Main method for tpserver-cpp
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "0.0.0"
#endif

#include "console.h"
#include "logging.h"
#include "game.h"
#include "net.h"
#include "settings.h"
#include "persistence.h"
#ifdef HAVE_LIBMYSQL
#include <modules/persistence/mysql/mysqlpersistence.h>
#endif

#include <modules/games/minisec/minisec.h>


int main(int argc, char **argv)
{

  Settings *mySettings = Settings::getSettings();
  mySettings->readArgs(argc, argv);

  if(mySettings->get("NEVER_START") != "!"){
    mySettings->readConfFile();

	Logger *myLogger = Logger::getLogger();

	myLogger->info("Tpserver-cpp " VERSION " starting");
	myLogger->info("This is GPL software, please see the COPYING file");

        try{

	Console *myConsole = Console::getConsole();
	myConsole->open();

	Game *myGame = Game::getGame();

            try{
                Persistence* myPersistence = 
#ifdef HAVE_LIBMYSQL
                    new MysqlPersistence();
#else
                    new Persistence();
#endif
                if(myPersistence->init()){
                    myLogger->debug("Persistence initialised");
                }else{
                    myLogger->error("Problem initialising Persistence");
                    throw std::exception();
                }
                myGame->setPersistence(myPersistence);

	//hack temp code
	myGame->setRuleset(new MiniSec());
	myGame->load();
	myGame->start();

	Network *myNetwork = Network::getNetwork();
	//temp code - should be removed when console is working fully
	myNetwork->start();
	//temp code end

	myNetwork->masterLoop();

	myNetwork->stop();
	myGame->saveAndClose();
                myPersistence->shutdown();
            }catch(std::exception e){
                myLogger->debug("Caught exception");
            }
	myConsole->close();

        }catch(std::exception e){
            myLogger->debug("Caught exception, exiting");
        }

	myLogger->info("TP-server exiting");
	myLogger->flush();

  }

  return 0;
}
