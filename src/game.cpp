#include <iostream>
#include <stdio.h>

#include "logging.h"
#include "player.h"
#include "object.h"

#include "game.h"

Game *Game::myInstance = NULL;

Game *Game::getGame()
{
	if (myInstance == NULL) {
		myInstance = new Game();
	}
	return myInstance;
}

void Game::createRandomUniverse()
{
	Logger::getLogger()->info("Creating random universe");
}

void Game::createRealUniverse()
{
	Logger::getLogger()->info("Creating real universe");
}

void Game::createTutorial()
{
	Logger::getLogger()->info("Creating tutorial");
	objects.clear();
	universe = new IGObject();
	objects.push_front(universe);
	universe->setSize(100000000000ll);
	universe->setType(0);
	universe->setName("The Universe");
	universe->setPosition3(0ll, 0ll, 0ll);
	universe->setVelocity3(0ll, 0ll, 0ll);
	universe->setAcceleration3(0ll, 0ll, 0ll);
	//add contained objects
	IGObject *mw_galaxy = new IGObject();
	objects.push_back(mw_galaxy);
	mw_galaxy->setSize(10000000000ll);
	mw_galaxy->setType(1);
	mw_galaxy->setName("Milky Way Galaxy");
	mw_galaxy->setPosition3(0ll, -6000ll, 0ll);
	mw_galaxy->setVelocity3(0ll, 1000ll, 0ll);
	mw_galaxy->setAcceleration3(0ll, 0ll, 0ll);
	universe->addContainedObject(mw_galaxy);
	// star system 1
	IGObject *sol = new IGObject();
	objects.push_back(sol);
	sol->setSize(1400000ll);
	sol->setType(2);
	sol->setName("Sol/Terra System");
	sol->setPosition3(3000000000ll, 2000000000ll, 0ll);
	sol->setVelocity3(-1500000ll, 1500000ll, 0ll);
	sol->setAcceleration3(-70000ll, -60000ll, 0ll);
	mw_galaxy->addContainedObject(sol);
	// star system 2
	IGObject *ac = new IGObject();
	objects.push_back(ac);
	ac->setSize(800000ll);
	ac->setType(2);
	ac->setName("Alpha Centauri System");
	ac->setPosition3(-1500000000ll, 1500000000ll, 0ll);
	ac->setVelocity3(-1000000ll, -1000000ll, 0ll);
	ac->setAcceleration3(70000ll, -60000ll, 0ll);
	mw_galaxy->addContainedObject(ac);
	// star system 3
	IGObject *sirius = new IGObject();
	objects.push_back(sirius);
	sirius->setSize(2000000ll);
	sirius->setType(2);
	sirius->setName("Sirius System");
	sirius->setPosition3(-250000000ll, -4000000000ll, 0ll);
	sirius->setVelocity3(2300000ll, 0ll, 0ll);
	sirius->setAcceleration3(0ll, -120000ll, 0ll);
	mw_galaxy->addContainedObject(sirius);

}

void Game::loadGame(char *file)
{
	FILE *infile = fopen(file, "r");
	if (infile != NULL) {
		Logger::getLogger()->debug("File opened");
		int n = 13;
		while (n == 12 || n == 13) {
			int id;
			int type;
			long long size;
			char *name;
			long long posx, posy, posz;
			long long velx, vely, velz;
			long long accx, accy, accz;
			int pid;
			name = new char[100];
			n = fscanf(infile, "%d%d%Ld%Ld%Ld%Ld%Ld%Ld%Ld%Ld%Ld%Ld%d", &id, &type, &size, &posx, &posy, &posz, &velx, &vely, &velz, &accx, &accy, &accz, &pid);
			if (n == 12 || n == 13) {
				fgets(name, 100, infile);
				int len = strlen(name);
				char *nl = name;
				nl += len - 1;
				if (*nl == '\n') {
					*nl = '\0';
				}
				IGObject *fileob = new IGObject();
				fileob->setID(id);
				fileob->setType(type);
				fileob->setSize(size);
				fileob->setName(name);
				fileob->setPosition3(posx, posy, posz);
				fileob->setVelocity3(velx, vely, velz);
				fileob->setAcceleration3(accx, accy, accz);
				if (n == 13) {
					IGObject *parent = getObject(pid);
					parent->addContainedObject(fileob);
				}
				objects.push_back(fileob);
				if (id == 0)
					universe = fileob;
				Logger::getLogger()->debug("Loaded an Object");
			} else {
				Logger::getLogger()->warning("Did not read whole line, discading and closing");
			}
			delete name;
		}
		fclose(infile);
	} else {
		Logger::getLogger()->warning("Could not open file");
	}

}

void Game::save()
{
	Logger::getLogger()->info("Game saved");
}

Player *Game::findPlayer(char *name, char *pass)
{
	Logger::getLogger()->debug("finding player");

	//look for current/known players
	Player *rtn = NULL;

	// hack HACK!!
	if (strcmp("guest", name) == 0 && strcmp("guest", pass) == 0)
		return rtn;
	// end of hack HACK!!

	std::list < Player * >::iterator itcurr, itend;

	itend = players.end();
	for (itcurr = players.begin(); itcurr != itend; itcurr++) {
		char *itname = (*itcurr)->getName();
		if (strncmp(name, itname, strlen(name) + 1) == 0) {
			char *itpass = (*itcurr)->getPass();
			if (strncmp(pass, itpass, strlen(pass) + 1) == 0) {
				rtn = (*itcurr);
			}
			delete itpass;
		}
		delete itname;
		if (rtn != NULL)
			break;
	}

	if (rtn == NULL) {
		//if new, create new player

		rtn = new Player();
		rtn->setName(name);
		rtn->setPass(pass);
		players.push_back(rtn);
	}
	return rtn;
}

IGObject *Game::getObject(unsigned int id)
{
	if (id == 0) {
		return universe;
	}
	IGObject *rtn = NULL;
	std::list < IGObject * >::iterator itcurr, itend;
	itend = objects.end();
	for (itcurr = objects.begin(); itcurr != itend; itcurr++) {
		if ((*itcurr)->getID() == id) {
			rtn = (*itcurr);
			break;
		}
	}
	return rtn;
	//may need more work
}

void Game::doEndOfTurn()
{
	Logger::getLogger()->info("End Of Turn started");
}

void Game::saveAndClose()
{
	save();
	//remove and delete players

	//remove and delete objects

	Logger::getLogger()->info("Game saved & closed");
}

Game::Game()
{

}

Game::Game(Game & rhs)
{

}

Game::~Game()
{

}

Game Game::operator=(Game & rhs)
{

}
