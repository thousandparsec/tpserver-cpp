#ifndef RESULT_H
#define RESULT_H


#include <string>

class Result{

 public:
  Result();
  Result(Result& rhs);
  ~Result();

  Result operator=(Result& rhs);

  String getDescription();
  
  void setDescription(String newdesc);

 private:
  String description;
  
};

#endif
