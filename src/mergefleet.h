#ifndef MERGEFLEET_H
#define MERGEFLEET_H

#include "order.h"

class Move;

class MergeFleet : public Order{
 public:
  MergeFleet();
  virtual ~MergeFleet();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f);
  
  bool doOrder(IGObject * ob);
  
  void describeOrder(Frame * f) const;
  Order* clone() const;

 private:
  int fleetid;
  Move * moveorder;

};

#endif
