#ifndef COLONISE_H
#define COLONISE_H

#include "order.h"

class Colonise : public Order {
 public:
  Colonise();
  virtual ~Colonise();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f);
  
  bool doOrder(IGObject * ob);
  
  static void describeOrder(int orderType, Frame * f);

 private:
  int planetid;


};

#endif
