#ifndef CONSOLE_H
#define CONSOLE_H


class Console{
  
 public:
  static Console* getConsole();
  
  void mainLoop();

  void close();
  

 private:
  Console(Console& rhs);
  Console operator=(Console& rhs);
  Console();
  ~Console();
  
  static Console *myInstance;

};

#endif
