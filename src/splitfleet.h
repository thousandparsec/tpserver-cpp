#ifndef SPLITFLEET_H
#define SPLITFLEET_H

#include "order.h"
#include <map>

class SplitFleet : public Order{
 public:
  SplitFleet();
  virtual ~SplitFleet();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f);
  
  bool doOrder(IGObject * ob);
  
  static void describeOrder(int orderType, Frame * f);
  
 private:
  std::map<int, int> ships;

};

#endif
