#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "logging.h"
#include "net.h"
#include "frame.h"

#include "connection.h"

Connection::Connection(){
  status = 0;
}

Connection::Connection(Connection &rhs){
  sockfd = rhs.sockfd;
  status = rhs.status;
}

Connection::Connection(int fd){
  sockfd = fd;
  status = 1;
  Network::getNetwork()->addFD(fd);
}

Connection::~Connection(){
  if(status != 0){
    close();
  }
}

Connection Connection::operator=(Connection &rhs){
  sockfd = rhs.sockfd;
  status = rhs.status;
}

int Connection::getFD(){
  return sockfd;
}

void Connection::setFD(int fd){
  sockfd = fd;
  status = 1;
  Network::getNetwork()->addFD(fd);
}

void Connection::process(){
  Logger::getLogger()->debug("About to Process");
  switch(status){
  case 1:
    //check if user is really a TP protocol ver1 client
    verCheck();
    break;
  case 2:
    //authorise the user
    login();
    break;
  case 3:
    //process as normal
    Logger::getLogger()->debug("Hit area that needs to be done"); //todo
    break;
  case 0:
  default:
    //do nothing
    Logger::getLogger()->warning("Tryed to process connections that is closed or invalid");
    if(status != 0)
      close();
    status = 0;
    break;
  }
  Logger::getLogger()->debug("Finished Processing");
}

void Connection::close(){
  Logger::getLogger()->debug("Closing connection");
  Network::getNetwork()->removeFD(sockfd);
  ::close(sockfd);
  status = 0;
}

void Connection::sendFrame(Frame* frame){
  char* packet = frame->getPacket();
  if(packet != NULL){
    int len = frame->getLength() + 12;
    send(sockfd, packet, len, 0);
    delete[] packet;
  }else{
    Logger::getLogger()->warning("Could not get packet from frame to send");
  }
  delete frame;
}

int Connection::getStatus(){
  return status;
}

void Connection::verCheck(){
  char* buff = new char[12];
  int len = read(sockfd, buff, 12);
  if(len == 12 && memcmp(buff, "TP01\0\0\0\0\0\0\0\0", 12) == 0){
    status = 2;
    Logger::getLogger()->info("Client has correct version of protocol");
    Frame* okframe = new Frame();
    okframe->setType(ft_OK);
    okframe->packString("Protocol check ok, continue");
    sendFrame(okframe);
  }else{
    Logger::getLogger()->warning("Client did not show correct version of protocol");
    // send "I don't understand" message
    if(len != 0){
      Frame* failframe = new Frame();
      failframe->setType(ft_Fail);
      failframe->packInt(0); // TODO - should be a const or enum, protocol error
      failframe->packString("You are not running the correct protocol");
      sendFrame(failframe);
    }
    close();
  }
  
  delete[] buff;
}

void Connection::login(){
  char* headerbuff = new char[12];
  Frame* recvframe = new Frame();
  int len = read(sockfd, headerbuff, 12);
  if(len == 12){
    if((len = recvframe->setHeader(headerbuff)) != -1){
      printf("%d\n", len);
      char* data = new char[len];
      int dlen = read(sockfd, data, len);
      if(len != dlen){
	//have to think about this.... what do we do?
	Logger::getLogger()->debug("Read data not the length needed");
      }
      recvframe->setData(data, dlen);
      delete[] data;
    }else{
      Logger::getLogger()->debug("Incorrect header");
      // protocol error
      Frame* failframe = new Frame();
      failframe->setType(ft_Fail);
      failframe->packInt(0); // TODO - should be a const or enum, protocol error
      failframe->packString("Protocol Error");
      sendFrame(failframe);
      close();
    }
  }else{
    Logger::getLogger()->debug("Did not read 12 bytes");
    if(len != 0){
      Frame* failframe = new Frame();
      failframe->setType(ft_Fail);
      failframe->packInt(0); // TODO - should be a const or enum, protocol error
      failframe->packString("Protocol Error");
      sendFrame(failframe);
    }
    close();
  }
  delete headerbuff;
  if(status == 0){
    Logger::getLogger()->warning("Client protocol error in login");
  }else{
    char* username = recvframe->unpackString();
    char* password = recvframe->unpackString();
    if(username != NULL && password != NULL){
      //authenicate
      Frame* okframe = new Frame();
      okframe->setType(ft_OK);
      okframe->packString("Welcome");
      sendFrame(okframe);
      
      status = 3;
    }else{
      Logger::getLogger()->debug("username or password == NULL");
      Frame* failframe = new Frame();
      failframe->setType(ft_Fail);
      failframe->packInt(1); // TODO - should be a const or enum, Login error
      failframe->packString("Login Error - no username or password");
      sendFrame(failframe);
      close();
    }
    
  }
  delete recvframe;
  
}
