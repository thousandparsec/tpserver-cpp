#ifndef BUILD_H
#define BUILD_H

#include <map>

#include "order.h"

class Build : public Order{
 public:
  Build();
  virtual ~Build();

  void createFrame(Frame *f, int objID, int pos);
  bool inputFrame(Frame *f);

  bool doOrder(IGObject *ob);

  void describeOrder(Frame *f) const;
  Order* clone() const;

 private:
  std::map<int, int> fleettype;
  int turnstogo;
};

#endif
