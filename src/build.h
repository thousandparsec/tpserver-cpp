#ifndef BUILD_H
#define BUILD_H

#include "order.h"

class Build : public Order{
 public:
  Build();
  virtual ~Build();

  void createFrame(Frame *f, int objID, int pos);
  bool inputFrame(Frame *f);

  bool doOrder(IGObject *ob);

  static void describeOrder(int orderType, Frame *f);

 private:
  int fleettype;
  int turnstogo;
};

#endif
