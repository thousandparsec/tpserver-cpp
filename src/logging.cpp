#include <iostream>

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

	doLogging(0, msg);
}

void Logger::info(char *msg, ...)
{

	doLogging(1, msg);
}

void Logger::warning(char *msg, ...)
{

	doLogging(2, msg);
}

void Logger::error(char *msg, ...)
{

	doLogging(3, msg);
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
	std::cout << level << " " << msg << "\n";
}
