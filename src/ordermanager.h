#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <map>

class Order;
class Frame;

class OrderManager{
 public:
  OrderManager();
  ~OrderManager();

  
  bool checkOrderType(int type);
  void describeOrder(int ordertype, Frame * f);
  Order* createOrder(int ot);

  void addOrderType(Order* prototype);

 private:
  std::map<int, Order*> prototypeStore;
  int nextType;


};

#endif
