#ifndef LOGGER_H
#define LOGGER_H

class Logger {

      public:
	static Logger *getLogger();

	void debug(char *msg, ...);
	void info(char *msg, ...);
	void warning(char *msg, ...);
	void error(char *msg, ...);

	//void addLog(Log* newlog);
	//void removeLog(Log* newlog);

	void flush();


      private:
	 Logger();
	~Logger();
	 Logger(Logger & rhs);
	Logger operator=(Logger & rhs);

	void doLogging(int level, char *msg);

	static Logger *myInstance;

};

#endif
