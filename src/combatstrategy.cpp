
#include "object.h"

#include "combatstrategy.h"

CombatStrategy::CombatStrategy(){
  c1 = NULL;
  c2 = NULL;
}

CombatStrategy::~CombatStrategy(){
}

void CombatStrategy::setCombatants(IGObject *a, IGObject *b){
  c1 = a;
  c2 = b;
}

bool CombatStrategy::isAliveCombatant1(){
  return (c1 != NULL);
}

bool CombatStrategy::isAliveCombatant2(){
  return (c2 != NULL);
}
