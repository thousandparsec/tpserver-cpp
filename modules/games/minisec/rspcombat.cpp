/*  Rock-Scissor-Paper combat
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string>

#include <tpserver/object.h>
#include "fleet.h"
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/playermanager.h>
#include <tpserver/prng.h>
#include "planet.h"
#include <tpserver/logging.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include "combatant.h"

#include "rspcombat.h"

RSPCombat::RSPCombat() : objectcache(){
  obT_Fleet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Fleet");
  obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");
}

RSPCombat::~RSPCombat(){
}


void RSPCombat::doCombat(std::map<uint32_t, std::set<uint32_t> > sides){
  std::set<uint32_t> listallobids;
  std::set<uint32_t> listallplayerids;
  
  std::map<uint32_t, Combatant*> fleetcache;
  
  for(std::map<uint32_t, std::set<uint32_t> >::iterator itmap = sides.begin(); itmap != sides.end(); ++itmap){
    std::set<uint32_t> theset = itmap->second;
    Combatant* f1 = new Combatant();
    f1->setOwner(itmap->first);
    for(std::set<uint32_t>::iterator itset = theset.begin(); itset != theset.end(); ++itset){
      listallobids.insert(*itset);
      IGObject* obj = Game::getGame()->getObjectManager()->getObject(*itset);
      objectcache[*itset] = obj;
      if(obj->getType() == obT_Fleet){
        Fleet* f2 = (Fleet*)(obj->getObjectData());
        std::map<uint32_t, uint32_t> shiplist = f2->getShips();
        for(std::map<uint32_t, uint32_t>::iterator itship = shiplist.begin(); itship != shiplist.end(); ++itship){
          f1->addShips(itship->first, itship->second);
        }
        f1->setDamage(f1->getDamage() + f2->getDamage());
      }else{
        std::set<uint32_t> playershiptypes = Game::getGame()->getPlayerManager()->getPlayer(itmap->first)->getPlayerView()->getUsableDesigns();
        std::set<uint32_t>::iterator itbs = playershiptypes.begin();
        itbs++;
        itbs++;
        f1->addShips(*itbs, 2);
        if(((Planet*)(obj->getObjectData()))->getResource(2) == 1){
          //two more for home planets
          f1->addShips(*itbs, 2);
        }
      }
    }
    listallplayerids.insert(itmap->first);
    fleetcache[itmap->first] = f1;
  }
  
  std::map<uint32_t, std::string> msgstrings;
  for(std::set<uint32_t>::iterator itplayer = listallplayerids.begin(); itplayer != listallplayerids.end(); ++itplayer){
    msgstrings[*itplayer] = "";
  }
  
  

  Random* random = Game::getGame()->getRandom();
  
  Logger::getLogger()->debug("Combat start");
  
  while(fleetcache.size() >= 2){
    int pos1, pos2;
    if(fleetcache.size() == 2){
      pos1 = 0;
      pos2 = 1;
    }else{
      pos1 = pos2 = random->getInRange(0, fleetcache.size() - 1);
      while(pos2 == pos1){
        pos2 = random->getInRange(0, fleetcache.size() - 1);
      }
    }
    
    std::map<uint32_t, Combatant*>::iterator itpa = fleetcache.begin();
    advance(itpa, pos1);
    std::map<uint32_t, Combatant*>::iterator itpb = fleetcache.begin();
    advance(itpb, pos2);
    
    Combatant* f1 = itpa->second;
    Combatant* f2 = itpb->second;
    
    uint32_t ownerid1 = f1->getOwner();
    uint32_t ownerid2 = f2->getOwner();
        
    int r1 = random->getInRange(0, 2);
    
    std::list<uint32_t> d1, d2;
    
    if(r1 == 0){
      //draw
      d1 = f1->firepower(true);
      d2 = f2->firepower(true);
    }else if(r1 == 1){
      //pa win
      d1 = f1->firepower(false);
      uint nscout = 0;
      for(std::list<uint32_t>::iterator itcurr = d1.begin(); itcurr != d1.end(); ++itcurr){
        if((*itcurr) == 0)
          nscout++;
      }
      if(nscout == d1.size() || random->getReal1() < ((double)((double)nscout / (double)(d1.size()))) ){
        msgstrings[ownerid1] += "Your Fleet escaped. ";
        for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
            msgit != msgstrings.end(); ++msgit){
          if(msgit->first == ownerid1)
            continue;
          msgit->second += "Their fleet of escaped. ";
        }
        resolveDamage(f1, sides[ownerid1]);
        delete f1;
        fleetcache.erase(itpa);
        continue;
      }
    }else{
      //pb win
      d2 = f2->firepower(false);
      if(d2.empty())
        Logger::getLogger()->debug("Empty shot list from fleet 2");
      uint nscout = 0;
      for(std::list<uint32_t>::iterator itcurr = d2.begin(); itcurr != d2.end(); ++itcurr){
        if((*itcurr) == 0)
          nscout++;
      }
      if(nscout == d2.size() || random->getReal1() < ((double)((double)nscout / (double)(d2.size()))) ){
        msgstrings[ownerid2] += "Your Fleet escaped. ";
        for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
            msgit != msgstrings.end(); ++msgit){
          if(msgit->first == ownerid2)
            continue;
          msgit->second += "Their fleet of escaped. ";
        }
        resolveDamage(f2, sides[ownerid2]);
        delete f2;
        fleetcache.erase(itpb);
        continue;
      }
    }

    Logger::getLogger()->debug("f1 hit by %d shots from d2", d2.size());
    
    if(!f1->hit(d2)){
      msgstrings[ownerid1] += "Your fleet was destroyed. ";
      msgstrings[ownerid2] += "You destroyed their fleet. ";
      for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
          msgit != msgstrings.end(); ++msgit){
        if(msgit->first == ownerid1 || msgit->first == ownerid2)
          continue;
        msgit->second += "An enemy destroyed an enemy fleet. ";
      }
      
      resolveDamage(f1, sides[ownerid1]);
      delete f1;
      fleetcache.erase(itpa);
    }
    
    Logger::getLogger()->debug("f2 hit by %d shots from d1", d1.size());
    
    if(!f2->hit(d1)){
      msgstrings[ownerid2] += "Your fleet was destroyed. ";
      msgstrings[ownerid1] += "You destroyed their fleet. ";
      for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
          msgit != msgstrings.end(); ++msgit){
        if(msgit->first == ownerid2 || msgit->first == ownerid1)
          continue;
        msgit->second += "An enemy destroyed an enemy fleet. ";
      }
      
      resolveDamage(f2, sides[ownerid2]);
      delete f2;
      fleetcache.erase(itpb);
    }

  }
  
  Combatant* flast = fleetcache.begin()->second;
  resolveDamage(flast, sides[flast->getOwner()]);
  delete flast;
  fleetcache.erase(fleetcache.begin());
  if(!fleetcache.empty()){
    Logger::getLogger()->warning("fleetcache not empty at end of combat");
  }
  
  for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
      msgit != msgstrings.end(); ++msgit){
    Message *msg;
    msg = new Message();
    msg->setSubject("Combat");
    for(std::set<uint32_t>::iterator itob = listallobids.begin(); itob != listallobids.end();
        ++itob){
      msg->addReference(rst_Object, *itob);
    }
    for(std::set<uint32_t>::iterator itpl = listallplayerids.begin();
        itpl != listallplayerids.end(); ++itpl){
      msg->addReference(rst_Player, *itpl);
    }
    
    msg->setBody(msgit->second);
    
    Game::getGame()->getPlayerManager()->getPlayer(msgit->first)->postToBoard(msg);
  }
  
  for(std::map<uint32_t, IGObject*>::iterator itob = objectcache.begin(); 
      itob != objectcache.end(); ++itob){
    Game::getGame()->getObjectManager()->doneWithObject(itob->first);
  }
  objectcache.clear();
  
}

void RSPCombat::resolveDamage(Combatant* fleet, std::set<uint32_t> objects){
  std::map<uint32_t, uint32_t> fleetlist = fleet->getShips();
  
  uint32_t temp;
  
  std::set<uint32_t> playershiptypes = Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->getPlayerView()->getUsableDesigns();
  std::set<uint32_t>::iterator itbs = playershiptypes.begin();
  temp = fleetlist[*itbs];
  itbs++;
  temp = fleetlist[*itbs];
  itbs++;
  temp = fleetlist[*itbs];
  uint32_t planetshiptype = *itbs;
  
  for(std::set<uint32_t>::iterator itob = objects.begin(); itob != objects.end();
      ++itob){
    IGObject* obj = objectcache[*itob];
    if(obj->getType() == obT_Fleet){
      ((Fleet*)(obj->getObjectData()))->setDamage(0);
    }
  }
  
  bool damageassigned = false;
  
  for(std::map<uint32_t, uint32_t>::reverse_iterator itship = fleetlist.rbegin(); 
      itship != fleetlist.rend(); ++itship){
    uint32_t shiptype = itship->first;
    uint32_t squant = itship->second;
    for(std::set<uint32_t>::iterator itob = objects.begin(); itob != objects.end();
        ++itob){
      IGObject* obj = objectcache[*itob];
      
      if(shiptype == planetshiptype && obj->getType() == obT_Planet){
        uint32_t shipquant = 2;
        bool ishomeplanet = (((Planet*)(obj->getObjectData()))->getResource(2) == 1);
        if(ishomeplanet)
          shipquant += 2;
        
        if(squant == 0){
          //planet has fallen
          if(ishomeplanet){
            ((Planet*)(obj->getObjectData()))->removeResource(2, 1);
          }
          uint32_t oldowner = ((Planet*)(obj->getObjectData()))->getOwner();
          ((Planet*)(obj->getObjectData()))->setOwner(0);
          uint32_t queueid = static_cast<OrderQueueObjectParam*>(obj->getObjectData()->getParameterByType(obpT_Order_Queue))->getQueueId();
          OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
          queue->removeOwner(oldowner);
          queue->removeAllOrders();
        }else{
          if(squant <= shipquant){
            squant = 0;
            fleet->setDamage(0);
            damageassigned = true;
          }else{
            uint32_t dmgship = fleet->getDamage()/squant;
            uint32_t dmgresd = fleet->getDamage()%squant;
            if(dmgresd > shipquant)
              dmgresd = shipquant;
            squant -= shipquant;
            fleet->setDamage(fleet->getDamage() - (shipquant * dmgship + dmgresd));
          }
        }
      }else if(obj->getType() == obT_Fleet){
        Fleet* f2 = (Fleet*)(obj->getObjectData());
        
        std::map<uint32_t, uint32_t> shiplist = f2->getShips();
        uint32_t shipquant = shiplist[shiptype];
        if(shipquant != 0){
          if(squant == 0){
            f2->removeShips(shiptype, shipquant);
          }else if(squant <= shipquant){
            f2->removeShips(shiptype, shipquant - squant);
            if(!damageassigned){
              fleet->setDamage(0);
              f2->setDamage(fleet->getDamage());
              damageassigned = true;
            }
            squant = 0;
            
          }else{
            if(!damageassigned){
              uint32_t dmgship = fleet->getDamage()/squant;
              uint32_t dmgresd = fleet->getDamage()%squant;
              if(dmgresd > shipquant)
                dmgresd = shipquant;
              squant -= shipquant;
              f2->setDamage(shipquant * dmgship + dmgresd);
              fleet->setDamage(fleet->getDamage() - f2->getDamage());
            }
            squant -= shipquant;
          }
        }
      }
    }
  }
  
  for(std::set<uint32_t>::iterator itob = objects.begin(); itob != objects.end();
      ++itob){
    IGObject* obj = objectcache[*itob];
    if(obj->getType() == obT_Fleet){
      if(((Fleet*)(obj->getObjectData()))->totalShips() == 0){
        Game::getGame()->getObjectManager()->scheduleRemoveObject(*itob);
      }
    }
  }
  
}
