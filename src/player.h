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

	char *getName();
	char *getPass();
	Connection *getConnection();

	void processIGFrame(Frame * frame);

      private:

	void processGetObject(Frame * frame);

	Connection *curConnection;
	char *name;
	char *passwd;

	 Player(Player & rhs);

	Player operator=(Player & rhs);


};

#endif
