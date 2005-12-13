/*  Logging for tpserver-cpp
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
#include <stdio.h>
#include <stdarg.h>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "settings.h"

#include "logging.h"

Logger *Logger::myInstance = NULL;

Logger *Logger::getLogger()
{
	if (myInstance == NULL) {
		myInstance = new Logger();
	}
	return myInstance;
}

void Logger::debug(char *msg, ...)
{
  char* fmsg = new char[100];
  va_list ap;
  va_start(ap, msg);
  int reallen = vsnprintf(fmsg, 100, msg, ap);
  if(reallen > 100){
    delete[] fmsg;
    fmsg = new char[reallen + 1];
    va_end(ap);
    va_start(ap, msg);
    vsnprintf(fmsg, reallen, msg, ap);
  }
  va_end(ap);
  doLogging(0, fmsg);
  delete[] fmsg;
}

void Logger::info(char *msg, ...)
{
 char* fmsg = new char[100];
  va_list ap;
  va_start(ap, msg);
  int reallen = vsnprintf(fmsg, 100, msg, ap);
  if(reallen > 100){
    delete[] fmsg;
    fmsg = new char[reallen + 1];
    va_end(ap);
    va_start(ap, msg);
    vsnprintf(fmsg, reallen, msg, ap);
  }
  va_end(ap);
  doLogging(1, fmsg);
  delete[] fmsg;
}

void Logger::warning(char *msg, ...)
{
  char* fmsg = new char[100];
  va_list ap;
  va_start(ap, msg);
  int reallen = vsnprintf(fmsg, 100, msg, ap);
  if(reallen > 100){
    delete[] fmsg;
    fmsg = new char[reallen + 1];
    va_end(ap);
    va_start(ap, msg);
    vsnprintf(fmsg, reallen, msg, ap);
  }
  va_end(ap);
  doLogging(2, fmsg);
  delete[] fmsg;
}

void Logger::error(char *msg, ...)
{
  char* fmsg = new char[100];
  va_list ap;
  va_start(ap, msg);
  int reallen = vsnprintf(fmsg, 100, msg, ap);
  if(reallen > 100){
    delete[] fmsg;
    fmsg = new char[reallen + 1];
    va_end(ap);
    va_start(ap, msg);
    vsnprintf(fmsg, reallen, msg, ap);
  }
  va_end(ap);
  doLogging(3, fmsg);
  delete[] fmsg;
  //exit(1);
}


void Logger::flush()
{
	info("Logger stopped");
}

void Logger::reconfigure(){
  loglevel = atoi(Settings::getSettings()->get("log_level").c_str());
}

Logger::Logger()
{
  reconfigure();
	info("Logger started");
}

Logger::~Logger()
{

}


void Logger::doLogging(int level, char *msg)
{
  if(level >= loglevel){
    switch(level){
    case 0:
      std::cout << "< Debug > ";
      break;
    case 1:
      std::cout << "< Info  > ";
      break;
    case 2:
      std::cout << "<Warning> ";
      break;
    case 3:
      std::cout << "< Error > ";
      break;
    default:
      std::cout << "<   " << level << "  > ";
    }
    std::cout << msg << std::endl;
  }
}
