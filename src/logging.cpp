#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
    vsnprintf(fmsg, reallen, msg, ap);
  }
  va_end(ap);
  doLogging(3, fmsg);
  delete[] fmsg;
  exit(1);
}


void Logger::flush()
{
	info("Logger stopped");
}


Logger::Logger()
{
	info("Logger started");
}

Logger::~Logger()
{

}


void Logger::doLogging(int level, char *msg)
{
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

  default:
    std::cout << "<   " << level << "  > ";
  }
  std::cout << msg << std::endl;
}
