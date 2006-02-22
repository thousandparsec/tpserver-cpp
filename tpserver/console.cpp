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
#include "settings.h"

#include "console.h"

Console *Console::myInstance = NULL;

static void linecomplete(char* line){
  Console::getConsole()->readLine(line);
}

static char** word_completion(const char* text, int start, int end){
  return Console::getConsole()->wordCompletion(text, start, end);
}

static char* command_generator(const char* text, int state){
  return Console::getConsole()->commandCompleter(text, state);
}

static char* turn_generator(const char* text, int state){
  return Console::getConsole()->turnCommandCompleter(text, state);
}

static char* network_generator(const char* text, int state){
  return Console::getConsole()->networkCommandCompleter(text, state);
}

static char* settings_generator(const char* text, int state){
    return Console::getConsole()->settingsCommandCompleter(text, state);
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

void Console::readLine(char* line){
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
    std::cout << "Turn\t\tTurn management functions" << std::endl;
    std::cout << "Network\t\tNetwork management functions" << std::endl;
    std::cout << "Settings\tSettings get and set functions" << std::endl;

    std::cout << std::endl;
  }else if(strncasecmp(line, "turn", 4) == 0){
    if(strncasecmp(line, "turn end", 8) == 0){
       Logger::getLogger()->info("End Of Turn initiated from console");
       Game::getGame()->doEndOfTurn();
       Game::getGame()->resetEOTTimer();
    }else if(strncasecmp(line, "turn time", 9) == 0){
      std::cout << Game::getGame()->secondsToEOT() << " seconds to EOT" << std::endl;
    }else if(strncasecmp(line, "turn reset", 10) == 0){
        Game::getGame()->resetEOTTimer();
        std::cout << "Reset EOT timer, " << Game::getGame()->secondsToEOT() 
                << " seconds to EOT" << std::endl;
    }else if(strncasecmp(line, "turn length ", 12) == 0){
        uint32_t tlen = atoi(line+12);
        Game::getGame()->setTurnLength(tlen);
        std::cout << "Setting turn length to " << tlen << " seconds" << std::endl;
    }else if(strncasecmp(line, "turn number", 11) == 0){
        std::cout << "The current turn number is " << Game::getGame()->getTurnNumber() << std::endl;
    }else{
      std::cout << line << " function not known" << std::endl;
    }
    
  }else if(strncasecmp(line, "network", 7) == 0){
    if(strncasecmp(line, "network start", 13) == 0){
      Network::getNetwork()->start();
    }else if(strncasecmp(line, "network stop", 12) == 0){
      Network::getNetwork()->stop();
    }else{
      std::cout << line << " function not known" << std::endl;
    }
  }else if(strncasecmp(line, "settings", 8) == 0){
        if(strncasecmp(line, "settings set", 12) == 0){
            std::string tmp1(line);
            tmp1 = tmp1.substr(13);
            size_t pos1 = tmp1.find(' ');
            Settings::getSettings()->set(tmp1.substr(0, pos1), tmp1.substr(pos1+1));
            std::cout << "Setting value of \"" << tmp1.substr(0, pos1) << "\" to \"" << tmp1.substr(pos1+1) << "\"." << std::endl;
        }else if(strncasecmp(line, "settings get", 12) == 0){
            std::string tmp = std::string(line).substr(13);
            std::cout << "Setting \"" << tmp << "\" is set to \"" << Settings::getSettings()->get(tmp) << "\"." << std::endl;
        }else{
            std::cout << line << " function not known" << std::endl;
        }
  }

  free(line);
#endif
}

char** Console::wordCompletion(const char* text, int start, int end){
#ifdef HAVE_LIBREADLINE
  char **matches;

  matches = (char **)NULL;

  commstate = 0;
  if (start == 0){
    matches = rl_completion_matches (text, command_generator);
  }
  else if(strncasecmp(rl_line_buffer, "turn", 4) == 0){
    matches = rl_completion_matches(text, turn_generator);
  }else if(strncasecmp(rl_line_buffer, "network", 7) == 0){
    matches = rl_completion_matches(text, network_generator);
  }else if(strncasecmp(rl_line_buffer, "settings", 8) == 0){
    matches = rl_completion_matches(text, settings_generator);
  }
  rl_attempted_completion_over = 1;

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
    if(strncasecmp(text, "quit", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "quit", 5);
      commstate = 1;
      break;
    }
  case 1:
    if(strncasecmp(text, "exit", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "exit", 5);
      commstate = 2;
      break;
    }
  case 2:
    if(strncasecmp(text, "help", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "help", 5);
      commstate = 3;
      break;
    }
  case 3:
    if(strncasecmp(text, "turn", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "turn", 5);
      commstate = 4;
      break;
    }
  case 4:
    if(strncasecmp(text, "network", strlen(text)) == 0){
      cname = (char*)malloc(8);
      strncpy(cname, "network", 8);
      commstate = 5;
      break;
    }
    case 5:
        if(strncasecmp(text, "settings", strlen(text)) == 0){
            cname = (char*)malloc(9);
            strncpy(cname, "settings", 9);
            commstate = 6;
            break;
        }
  default:
    commstate = 9;
    break;
  }
  return cname;
}

char* Console::turnCommandCompleter(const char* text, int state){
  if(state == 0){
    commstate = 0;
  }
  char* cname = NULL;
  switch(commstate){
  case 0:
    if(strncasecmp(text, "end", strlen(text)) == 0){
      cname = (char*)malloc(4);
      strncpy(cname, "end", 4);
      commstate = 1;
      break;
    }
  case 1:
    if(strncasecmp(text, "time", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "time", 5);
      commstate = 2;
      break;
    }
      case 2:
          if(strncasecmp(text, "reset", strlen(text)) == 0){
              cname = (char*)malloc(6);
              strncpy(cname, "reset", 6);
              commstate = 3;
              break;
          }
      case 3:
          if(strncasecmp(text, "length", strlen(text)) == 0){
              cname = (char*)malloc(7);
              strncpy(cname, "length", 7);
              commstate = 4;
              break;
          }
      case 4:
          if(strncasecmp(text, "number", strlen(text)) == 0){
              cname = (char*)malloc(7);
              strncpy(cname, "number", 7);
              commstate = 5;
              break;
          }
  default:
    commstate = 9;
    break;
  }
  return cname;
}

char* Console::networkCommandCompleter(const char* text, int state){
  if(state == 0){
    commstate = 0;
  }
  char* cname = NULL;
  switch(commstate){
  case 0:
    if(strncasecmp(text, "start", strlen(text)) == 0){
      cname = (char*)malloc(6);
      strncpy(cname, "start", 6);
      commstate = 1;
      break;
    }
  case 1:
    if(strncasecmp(text, "stop", strlen(text)) == 0){
      cname = (char*)malloc(5);
      strncpy(cname, "stop", 5);
      commstate = 2;
      break;
    }
  default:
    commstate = 9;
    break;
  }
  return cname;
}

char* Console::settingsCommandCompleter(const char* text, int state){
  if(state == 0){
    commstate = 0;
  }
  char* cname = NULL;
  switch(commstate){
  case 0:
    if(strncasecmp(text, "set", strlen(text)) == 0){
      cname = (char*)malloc(4);
      strncpy(cname, "set", 4);
      commstate = 1;
      break;
    }
  case 1:
    if(strncasecmp(text, "get", strlen(text)) == 0){
      cname = (char*)malloc(4);
      strncpy(cname, "get", 4);
      commstate = 2;
      break;
    }
  default:
    commstate = 9;
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
