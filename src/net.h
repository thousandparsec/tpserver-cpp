#ifndef NET_H
#define NET_H

#include <pthread.h>
#include <map>
#include <sys/select.h>

class Connection;
class Frame;

class Network {

      public:
	static Network *getNetwork();

	//stuff

	void addFD(int fd);
	void removeFD(int fd);

	void start();

	void stop();

	void sendToAll(Frame * frame);

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

	 std::map < int, Connection * >connections;


};

#endif
