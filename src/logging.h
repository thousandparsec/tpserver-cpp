#ifndef LOGGER_H
#define LOGGER_H

class Logger {

      public:
	static Logger *getLogger();
	
	// char *
	void debug(char *msg, ...);
	void info(char *msg, ...);
	void warning(char *msg, ...);
	void error(char *msg, ...);

	// int
	void debug(int i);
	void info(int i);
	void warning(int i);
	void error(int i);
	
	//void addLog(Log* newlog);
	//void removeLog(Log* newlog);

	void flush();


      private:
	 Logger();
	~Logger();
	 Logger(Logger & rhs);
	Logger operator=(Logger & rhs);

	void doLogging(int level, char *msg);
	void doLogging(int level, int i);

	static Logger *myInstance;

};

#endif
