#ifndef COMBATSTRATEGY_H
#define COMBATSTRATEGY_H

class IGObject;

class CombatStrategy{
 public:
  CombatStrategy();
  virtual ~CombatStrategy();

  void setCombatants(IGObject * a, IGObject * b);
  bool isAliveCombatant1();
  bool isAliveCombatant2();

  virtual void doCombat() = 0;

 protected:
  IGObject *c1, *c2;
  
};

#endif
