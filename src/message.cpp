

#include "frame.h"

#include "message.h"

void Message::setType(int ntype){
  msgtype = ntype;
}

int Message::getType(){
  return msgtype;
}

void Message::setSubject(const std::string & nsub){
  subject = nsub;
}

std::string Message::getSubject(){
  return subject;
}

void Message::setBody(const std::string & nbody){
  body = nbody;
}

std::string Message::getBody(){
  return body;
}

void Message::pack(Frame * frame){
  frame->packInt(1);
  frame->packInt(msgtype);
  frame->packString(subject.c_str());
  frame->packString(body.c_str());
}
