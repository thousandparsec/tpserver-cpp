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

void Logger::debug(int i)
{

	doLogging(0, i);
}

void Logger::info(int i)
{

	doLogging(1, i);
}

void Logger::warning(int i)
{

	doLogging(2, i);
}

void Logger::error(int i)
{

	doLogging(3, i);
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

void Logger::doLogging(int level, int i)
{
	std::cout << level << " " << i << "\n";
}

void Logger::doLogging(int level, char *msg)
{
	std::cout << level << " " << msg << "\n";
}
