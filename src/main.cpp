#include <iostream>

#include "console.h"
#include "logging.h"
#include "game.h"
#include "net.h"
#include "settings.h"

void parseOptions(int argc, char **argv)
{
	Settings *mySettings = Settings::getSettings();
	mySettings->readArgs(argc, argv);
}


int main(int argc, char **argv)
{
	Logger *myLogger = Logger::getLogger();

	parseOptions(argc, argv);

	myLogger->info("TP-server starting");

	Console *myConsole = Console::getConsole();

	Game *myGame = Game::getGame();
	//hack temp code
	myGame->createTutorial();

	Network *myNetwork = Network::getNetwork();
	//temp code - should be removed when console is working fully
	myNetwork->start();
	//temp code end

	myConsole->mainLoop();

	myNetwork->stop();
	myGame->saveAndClose();
	myConsole->close();

	myLogger->info("TP-server exiting");
	myLogger->flush();

	return 0;
}
