#ifndef MERGEFLEET_H
#define MERGEFLEET_H

#include "order.h"

class MergeFleet : public Order{
 public:
  MergeFleet();
  virtual ~MergeFleet();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f);
  
  bool doOrder(IGObject * ob);
  
  static void describeOrder(int orderType, Frame * f);

 private:
  int fleetid;

};

#endif
