#ifndef COLONISE_H
#define COLONISE_H

#include "order.h"

class Move;

class Colonise : public Order {
 public:
  Colonise();
  virtual ~Colonise();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f);
  
  bool doOrder(IGObject * ob);
  
  void describeOrder(Frame * f) const;
  Order* clone() const;

 private:
  int planetid;
  Move* moveorder;


};

#endif
