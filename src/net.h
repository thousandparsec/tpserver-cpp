#ifndef NET_H
#define NET_H

#include <pthread.h>
#include <list>
#include <sys/select.h>

class Connection;

class Network {

      public:
	static Network *getNetwork();

	//stuff

	void addFD(int fd);
	void removeFD(int fd);

	void start();

	void stop();

	// don't you even think about calling these functions

	void masterLoop();


      private:
	 Network();
	~Network();
	 Network(Network & rhs);
	Network operator=(Network & rhs);

	static Network *myInstance;

	int serverFD;
	fd_set master_set;
	int max_fd;
	bool active;

	pthread_t master;


	bool halt;

	 std::list < Connection * >connections;


};

#endif
