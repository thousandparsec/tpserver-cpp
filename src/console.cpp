/*  Server terminal console for server
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <iostream>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "logging.h"
#include "net.h"
#include "game.h"
#include "object.h"

#include "console.h"

Console *Console::myInstance = NULL;

static void linecomplete(char* line){
  Console::getConsole()->completeLine(line);
}

static char** word_completion(const char* text, int start, int end){
  return Console::getConsole()->wordCompletion(text, start, end);
}

static char* command_generator(const char* text, int state){
  return Console::getConsole()->commandCompleter(text, state);
}

Console *Console::getConsole()
{
	if (myInstance == NULL)
		myInstance = new Console();
	return myInstance;
}

void Console::open(){
  Network::getNetwork()->addConnection(this);
#ifdef HAVE_LIBREADLINE
  rl_callback_handler_install("", linecomplete);
  rl_attempted_completion_function = word_completion;
#endif
  Logger::getLogger()->info("Console ready");
}

void Console::process(){

#ifndef HAVE_LIBREADLINE
  // this is going to need a lot of work
  char key;
  int num;
  num = scanf("%c", &key);
  if (num != 1 || key == 'q')
    Network::getNetwork()->stopMainLoop();
  if (key == 'h') {
    std::cout << "q to quit" << std::endl;
    std::cout << "h for help" << std::endl;
    std::cout << "t to end turn" << std::endl;
    std::cout << "T to get seconds until end of turn" << std::endl;
    std::cout << "n to stop network" << std::endl;
    std::cout << "N to start network" << std::endl;
  }
  if (key == 't') {
    Logger::getLogger()->info("End Of Turn initiated from console");
    Game::getGame()->doEndOfTurn();
    Game::getGame()->resetEOTTimer();
    
  }
  if(key == 'T'){
    std::cout << Game::getGame()->secondsToEOT() << " seconds to EOT" << std::endl;
  }
  if (key == 'n') {
    Network::getNetwork()->stop();
  }
  if (key == 'N') {
    Network::getNetwork()->start();
  }
  /*
    if (key == 'l') {
    char *file = new char[100];
    num = scanf("%s", file);
    if (num == 1) {
    Game::getGame()->loadGame(file);
    }
    }
    if (key == 'o') {
    int player, object, order;
    num = scanf("%d %d %d", &player, &object, &order);
    if (num == 3) {
    if (Game::getGame()->getObject(object)->addAction(-1, player, (OrderType) order)) {
    Logger::getLogger()->info("Extra action enabled");
    } else {
    Logger::getLogger()->info("Extra action not added");
    }
    } else {
    Logger::getLogger()->warning("Did not get the parameters");
    }
    }
  */
#else
  rl_callback_read_char();
#endif
}

void Console::close()
{
  Network::getNetwork()->removeConnection(this);
#ifdef HAVE_LIBREADLINE
  rl_callback_handler_remove();
#endif
  Logger::getLogger()->info("Console closed");
}

void Console::completeLine(char* line){
#ifdef HAVE_LIBREADLINE
  if(line != NULL){
    add_history(line);
  }
  if(line == NULL || strncasecmp(line, "quit", 4) == 0 || strncasecmp(line, "exit", 4) == 0){
    Network::getNetwork()->stopMainLoop();
  }else if(strncasecmp(line, "help", 4) == 0){
    std::cout << std::endl << "Commands:" << std::endl;
    std::cout << "Quit, Exit\tQuit and shutdown the server" << std::endl;
    std::cout << "Help\t\tThis help" << std::endl;

    std::cout << std::endl;
  }

  free(line);
#endif
}

char** Console::wordCompletion(const char* text, int start, int end){
#ifdef HAVE_LIBREADLINE
  char **matches;

  matches = (char **)NULL;

  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
#endif
}

char* Console::commandCompleter(const char* text, int state){
  if(state == 0){
    commstate = 0;
  }
  char* cname = NULL;
  switch(commstate){
  case 0:
    if(strncmp(text, "quit", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "quit", 5);
      commstate = 1;
      break;
    }
  case 1:
    if(strncmp(text, "exit", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "exit", 5);
      commstate = 2;
      break;
    }
  case 2:
    if(strncmp(text, "help", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "help", 5);
      commstate = 3;
      break;
    }
  default:
    commstate = 3;
    break;
  }
  return cname;
}

Console::Console() : Connection()
{
	Logger::getLogger()->info("Console opened");
	status = 1;
	sockfd = 2;
}

Console::~Console()
{

}
