#ifndef RSPCOMBAT_H
#define RSPCOMBAT_H

#include "combatstrategy.h"

class RSPCombat : public CombatStrategy{
 public:
  RSPCombat();
  virtual ~RSPCombat();

  void doCombat();


};

#endif
