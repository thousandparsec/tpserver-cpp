

#include "frame.h"
#include "message.h"

#include "board.h"


void Board::setBoardID(int i){
  boardid = i;
}

int Board::getBoardID(){
  return boardid;
}

void Board::setName(const std::string & nname){
  name = nname;
}

std::string Board::getName(){
  return name;
}

void Board::setDescription(const std::string & ndest){
  description = ndest;
}

std::string Board::getDescription(){
  return description;
}

void Board::addMessage(Message * msg, int pos){
  if(pos == -1){
    messages.push_back(msg);
  }else{
    std::list<Message*>::iterator itpos = messages.begin();
    advance(itpos, pos);
    messages.insert(itpos, msg);
  }
}

bool Board::removeMessage(unsigned int pos){
  if(pos >= messages.size() || pos < 0){
    return false;
  }
  std::list<Message*>::iterator itpos = messages.begin();
  advance(itpos, pos);
  delete (*itpos);
  messages.erase(itpos);
  return true;
}

void Board::packBoard(Frame * frame){
  frame->setType(ft02_Board);
  frame->packInt(boardid);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packInt(messages.size());
}

void Board::packMessage(Frame * frame, unsigned int msgnum){
  if(msgnum < messages.size()){
    frame->setType(ft02_Message);
    frame->packInt(boardid);
    frame->packInt(msgnum);
    std::list<Message*>::iterator itpos = messages.begin();
    advance(itpos, msgnum);
    (*itpos)->pack(frame);
  }else{
    frame->createFailFrame(fec_NonExistant, "No such Message on board");
  }
}
