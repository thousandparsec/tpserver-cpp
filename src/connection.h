#ifndef CONNECTION_H
#define CONNECTION_H

class Frame;

class Connection{

 public:
  
  Connection();
  Connection(Connection &rhs);
  Connection(int fd);
  ~Connection();
  Connection operator=(Connection &rhs);

  int getFD();
  void setFD(int fd);

  void process();
  void close();
  void sendFrame(Frame* frame);

  int getStatus();

 private:
  
  void verCheck();
  void login();
  
  int sockfd;
  // Player* player;
  int status;

};

#endif
