#ifndef BOARD_H
#define BOARD_H

#include <list>
#include <string>

class Message;
class Frame;

class Board{

 public:
  
  void setBoardID(int i);
  int getBoardID();

  void setName(const std::string & nname);
  std::string getName();

  void setDescription(const std::string & ndest);
  std::string getDescription();
  
  void addMessage(Message* msg, int pos);
  bool removeMessage(unsigned int pos);


  void packBoard(Frame * frame);
  void packMessage(Frame * frame, unsigned int msgnum);

 private:
  std::list<Message*> messages;

  int boardid;
  std::string name;
  std::string description;

};

#endif
