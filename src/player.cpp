#include "string.h"

#include "connection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"

#include "player.h"

int Player::nextpid = 1;

Player::Player()
{
	curConnection = NULL;
	name = NULL;
	passwd = NULL;
	pid = nextpid++;
}

Player::~Player()
{
	if (name != NULL) {
		delete[]name;
	}
	if (passwd != NULL) {
		delete[]passwd;
	}
	if (curConnection != NULL) {
		curConnection->close();
	}
}

void Player::setName(char *newname)
{
	if (name != NULL) {
		delete[]name;
	}
	int len = strlen(newname) + 1;
	name = new char[len];
	strncpy(name, newname, len);
}

void Player::setPass(char *newpass)
{
	if (passwd != NULL) {
		delete[]passwd;
	}
	int len = strlen(newpass) + 1;
	passwd = new char[len];
	strncpy(passwd, newpass, len);
}

void Player::setConnection(Connection * newcon)
{
	curConnection = newcon;
}

void Player::setID(int newid)
{
	pid = newid;
}

char *Player::getName()
{
	int len = strlen(name) + 1;
	char *temp = new char[len];
	strncpy(temp, name, len);
	return temp;
}

char *Player::getPass()
{
	int len = strlen(passwd) + 1;
	char *temp = new char[len];
	strncpy(temp, passwd, len);
	return temp;
}

Connection *Player::getConnection()
{
	return curConnection;
}

int Player::getID()
{
	return pid;
}

void Player::processIGFrame(Frame * frame)
{
	switch (frame->getType()) {
	case ft_Get_Object:
		processGetObject(frame);
		break;
		//case ft_Get_Order:
	default:
		Logger::getLogger()->warning("Player: Discarded frame, not processed");
		break;
	}

	delete frame;
}

void Player::processGetObject(Frame * frame)
{
	Logger::getLogger()->debug("doing get frame");
	if (frame->getLength() >= 4) {
		unsigned int objectID = frame->unpackInt();
		Frame *obframe = new Frame();
		Game::getGame()->getObject(objectID)->createFrame(obframe, pid);
		curConnection->sendFrame(obframe);
	}
}
