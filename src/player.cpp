#include "string.h"

#include "connection.h"
#include "frame.h"
#include "logging.h"

#include "player.h"

Player::Player(){
  curConnection = NULL;
  name = NULL;
  passwd = NULL;
}

Player::~Player(){
  if(name != NULL){
    delete[] name;
  }
  if(passwd != NULL){
    delete[] passwd;
  }
  if(curConnection != NULL){
    curConnection->close();
  }
}

void Player::setName(char* newname){
  if(name != NULL){
    delete[] name;
  }
  int len = strlen(newname) + 1;
  name = new char[len];
  strncpy(name, newname, len);
}

void Player::setPass(char* newpass){
  if(passwd != NULL){
    delete[] passwd;
  }
  int len = strlen(newpass) + 1;
  passwd = new char[len];
  strncpy(passwd, newpass, len);
}

void Player::setConnection(Connection* newcon){
  if(curConnection != NULL){
    curConnection->close();
  }
  curConnection = newcon;
}

char* Player::getName(){
  int len = strlen(name) + 1;
  char* temp = new char[len];
  strncpy(temp, name, len);
  return temp;
}

char* Player::getPass(){
  int len = strlen(passwd) + 1;
  char* temp = new char[len];
  strncpy(temp, passwd, len);
  return temp;
}

Connection* Player::getConnection(){
  return curConnection;
}

void Player::processIGFrame(Frame* frame){
  Logger::getLogger()->warning("Player: Discarded frame, not processed");

  delete frame;
}
