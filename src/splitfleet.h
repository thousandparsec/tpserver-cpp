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
  
  void describeOrder(Frame * f) const;
  Order* clone() const;
  
 private:
  std::map<int, int> ships;

};

#endif
