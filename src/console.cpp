/*  Server terminal console for server
 *
 *  Copyright (C) 2003-2004  Lee Begg and the Thousand Parsec Project
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

#include <stdio.h>

#include "logging.h"
#include "net.h"
#include "game.h"
#include "object.h"

#include "console.h"

Console *Console::myInstance = NULL;

Console *Console::getConsole()
{
	if (myInstance == NULL)
		myInstance = new Console();
	return myInstance;
}

void Console::mainLoop()
{
	Logger::getLogger()->info("Console ready");
	while (true) {
		// this is going to need a lot of work
		char key;
		int num;
		num = scanf("%c", &key);
		if (num != 1)
			break;
		if (key == 'q')
			break;
		if (key == 'h') {
		  std::cout << "q to quit" << std::endl;
		  std::cout << "h for help" << std::endl;
		  std::cout << "t to end turn" << std::endl;
		  std::cout << "T to get seconds until end of turn" << std::endl;
		  std::cout << "n to stop network" << std::endl;
		  std::cout << "N to start network" << std::endl;
		}
		if (key == 't') {
			Logger::getLogger()->info("End Of Turn initiated from console");
			Game::getGame()->doEndOfTurn();
			Game::getGame()->resetEOTTimer();

		}
		if(key == 'T'){
		  std::cout << Game::getGame()->secondsToEOT() << " seconds to EOT" << std::endl;
		}
		if (key == 'n') {
			Network::getNetwork()->stop();
		}
		if (key == 'N') {
			Network::getNetwork()->start();
		}
		/*
		if (key == 'l') {
			char *file = new char[100];
			num = scanf("%s", file);
			if (num == 1) {
				Game::getGame()->loadGame(file);
			}
		}
		if (key == 'o') {
			int player, object, order;
			num = scanf("%d %d %d", &player, &object, &order);
			if (num == 3) {
				if (Game::getGame()->getObject(object)->addAction(-1, player, (OrderType) order)) {
					Logger::getLogger()->info("Extra action enabled");
				} else {
					Logger::getLogger()->info("Extra action not added");
				}
			} else {
				Logger::getLogger()->warning("Did not get the parameters");
			}
		}
		*/
	}
	Logger::getLogger()->info("Server starting shutdown");
}

void Console::close()
{
	Logger::getLogger()->info("Console closed");
}

Console::Console()
{
	Logger::getLogger()->info("Console opened");
}

Console::~Console()
{

}
