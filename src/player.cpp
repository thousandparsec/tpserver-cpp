#include "string.h"

#include "connection.h"
#include "frame.h"
#include "logging.h"
#include "game.h"
#include "object.h"
#include "order.h"

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
	case ft_Get_Order:
		processGetOrder(frame);
		break;
	case ft_Add_Order:
		processAddOrder(frame);
		break;
	case ft_Remove_Order:
		processRemoveOrder(frame);
		break;
	case ft_Describe_Order:
		processDescribeOrder(frame);
		break;
	case ft_Get_Outcome:
		processGetOutcome(frame);
		break;
		//more
	default:
		Logger::getLogger()->warning("Player: Discarded frame, not processed");
		break;
	}

	delete frame;
}

void Player::processGetObject(Frame * frame)
{
	Logger::getLogger()->debug("doing get object frame");
	if (frame->getLength() >= 4) {
		unsigned int objectID = frame->unpackInt();
		Frame *obframe = new Frame();
		Game::getGame()->getObject(objectID)->createFrame(obframe, pid);
		curConnection->sendFrame(obframe);
	}
}

void Player::processGetOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing get order frame");
	Frame *of = new Frame();
	if (frame->getLength() >= 8) {
		unsigned int objectID = frame->unpackInt();
		int ordpos = frame->unpackInt();
		Order *ord = Game::getGame()->getObject(objectID)->getOrder(ordpos, pid);
		if (ord != NULL) {
			ord->createFrame(of, objectID, ordpos);
		} else {
			of->createFailFrame(9, "Could not get Order");
		}
	} else {
		of->createFailFrame(4, "Invalid frame");
	}
	curConnection->sendFrame(of);
}

void Player::processAddOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing add order frame");
	Frame *of = new Frame();
	if (frame->getLength() >= 8) {
		unsigned int objectID = frame->unpackInt();
		Order *ord = Order::createOrder((OrderType) frame->unpackInt());
		int pos = frame->unpackInt();
		ord->inputFrame(frame);
		if (Game::getGame()->getObject(objectID)->addOrder(ord, pos, pid)) {
			of->setType(ft_OK);
			of->packString("Order Added");
		} else {
			of->createFailFrame(19, "Could not add order");
		}
	} else {
		of->createFailFrame(4, "Invalid frame");
	}
	curConnection->sendFrame(of);
}

void Player::processRemoveOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing remove order frame");
	Frame *of = new Frame();
	if (frame->getLength() >= 8) {
		unsigned int objectID = frame->unpackInt();
		int ordpos = frame->unpackInt();
		if (Game::getGame()->getObject(objectID)->removeOrder(ordpos, pid)) {
			of->setType(ft_OK);
			of->packString("Order removed");
		} else {
			of->createFailFrame(13, "Could not remove Order");
		}
	} else {
		of->createFailFrame(4, "Invalid frame");
	}
	curConnection->sendFrame(of);
}

void Player::processDescribeOrder(Frame * frame)
{
	Logger::getLogger()->debug("doing describe order frame");
	Frame *of = new Frame();
	int ordertype = frame->unpackInt();
	Order::describeOrder(ordertype, of);
	curConnection->sendFrame(of);
}

void Player::processGetOutcome(Frame * frame)
{
	Logger::getLogger()->debug("doing get outcome");
	Frame *of = new Frame();
	if (frame->getLength() >= 8) {
		unsigned int objectID = frame->unpackInt();
		int ordpos = frame->unpackInt();
		Order *ord = Game::getGame()->getObject(objectID)->getOrder(ordpos, pid);
		if (ord != NULL) {
			ord->createOutcome(of, objectID, ordpos);
		} else {
			of->createFailFrame(19, "Could not get Outcome");
		}
	} else {
		of->createFailFrame(4, "Invalid frame");
	}
	curConnection->sendFrame(of);

}
