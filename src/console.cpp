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
			printf("q to quit\nh for help\nt to end turn\nn to stop network\nN to start network\nl to type file to load\no to add ordertype, object player\n");
		}
		if (key == 't') {
			Logger::getLogger()->info("End Of Turn started");

		}
		if (key == 'n') {
			Network::getNetwork()->stop();
		}
		if (key == 'N') {
			Network::getNetwork()->start();
		}
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
