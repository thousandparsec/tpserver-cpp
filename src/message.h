#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

class Frame;

class Message{

 public:

  void setType(int ntype);
  int getType();

  void setSubject(const std::string &nsub);
  std::string getSubject();
  
  void setBody(const std::string &nbody);
  std::string getBody();

  void pack(Frame * frame);

 private:
  int msgtype;
  std::string subject;
  std::string body;

};

#endif
