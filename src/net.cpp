#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

#include "logging.h"
#include "settings.h"
#include "connection.h"

#include "net.h"

// thread start function
static void *startMaster(void *arg)
{
	Network::getNetwork()->masterLoop();
	return NULL;
}


// Network Class methods

Network *Network::myInstance = NULL;


Network *Network::getNetwork()
{
	if (myInstance == NULL) {
		myInstance = new Network();
	}
	return myInstance;
}

void Network::addFD(int fd)
{
	Logger::getLogger()->debug("Adding a file descriptor");
	FD_SET(fd, &master_set);
	if (max_fd < fd) {
		max_fd = fd;
	}
}

void Network::removeFD(int fd)
{
	Logger::getLogger()->debug("Removing a file descriptor");
	FD_CLR(fd, &master_set);
	if (max_fd == fd) {
		Logger::getLogger()->debug("Changing max_fd");
		max_fd = serverFD;
		std::map < int, Connection * >::iterator itcurr, itend;
		itend = connections.end();
		for (itcurr = connections.begin(); itcurr != itend; itcurr++) {
			if (FD_ISSET((*itcurr).first, &master_set)) {
				if (max_fd < (*itcurr).first)
					max_fd = (*itcurr).first;
			}
		}

	}
}

void Network::start()
{
	if (active == true) {
		Logger::getLogger()->warning("Network already running");
		return;
	}
	Logger::getLogger()->info("Starting Network");

	serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD == -1)
		Logger::getLogger()->error("Could not create Socket");
	struct sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(6923);	// fix, make setting item
	myAddr.sin_addr.s_addr = INADDR_ANY;	// fix, make setting item
	memset(&(myAddr.sin_zero), '\0', 8);

	if (bind(serverFD, (struct sockaddr *) &myAddr, sizeof(struct sockaddr)) != 0) {
		perror("bind");
		Logger::getLogger()->error("Failed to bind to port and address");
	}

	if (listen(serverFD, 5) != 0)
		Logger::getLogger()->error("Failed to listen");

	FD_ZERO(&master_set);
	FD_SET(serverFD, &master_set);
	max_fd = serverFD;

	// time to create thread
	halt = false;
	pthread_create(&master, NULL, startMaster, NULL);



	active = true;
}


void Network::stop()
{
	if (active) {
		Logger::getLogger()->info("Stopping Network");
		close(serverFD);

		//more to come
		halt = true;
		pthread_join(master, NULL);

		active = false;

	} else {
		Logger::getLogger()->warning("Network already stopped");
	}
}


void Network::masterLoop()
{
	struct timeval tv;
	fd_set cur_set;
	while (!halt) {

		//sleep(1);

		cur_set = master_set;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		if (select(max_fd + 1, &cur_set, NULL, NULL, &tv) != 0) {

			if (FD_ISSET(serverFD, &cur_set)) {
				Logger::getLogger()->info("Accepting new connection");
				Connection *temp = new Connection(accept(serverFD, NULL, 0));
				connections[temp->getFD()] = temp;
			}

			std::map < int, Connection * >::iterator itcurr;
			for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
				if (FD_ISSET((*itcurr).first, &cur_set)) {
					(*itcurr).second->process();
				}
				if ((*itcurr).second->getStatus() == 0) {
					delete(*itcurr).second;
					std::map < int, Connection * >::iterator temp;
					temp = itcurr;
					--temp;
					connections.erase(itcurr);
					itcurr = temp;
				}
			}

		}


	}

	while (!connections.empty()) {
		(*connections.begin()).second->close();
		delete(*connections.begin()).second;
		connections.erase(connections.begin());
	}


}




Network::Network()
{



	halt = false;
	active = false;
}


Network::~Network()
{




}


Network::Network(Network & rhs)
{
}


Network Network::operator=(Network & rhs)
{
}
