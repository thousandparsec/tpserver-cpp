#ifndef PLAYER_H
#define PLAYER_H

class Connection;
class Frame;

class Player {
      public:
	Player();
	~Player();

	void setName(char *newname);
	void setPass(char *newpass);
	void setConnection(Connection * newcon);
	void setID(int newid);

	char *getName();
	char *getPass();
	Connection *getConnection();
	int getID();

	void processIGFrame(Frame * frame);

      private:

	static int nextpid;

	void processGetObject(Frame * frame);
	void processGetOrder(Frame * frame);
	void processAddOrder(Frame * frame);
	void processRemoveOrder(Frame * frame);
	void processDescribeOrder(Frame * frame)

	Connection *curConnection;
	char *name;
	char *passwd;
	int pid;

	 Player(Player & rhs);

	Player operator=(Player & rhs);


};

#endif
