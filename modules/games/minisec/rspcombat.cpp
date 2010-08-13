/*  Rock-Scissor-Paper combat
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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
#include <vector>
#include <algorithm>
#include <boost/format.hpp>

#include <tpserver/object.h>
#include "fleet.h"
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
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
#include <tpserver/designstore.h>
#include <tpserver/design.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/battlexml/battlelogger.h>
#include "combatant.h"

#include "rspcombat.h"

// CombatantSorter, for sorting the combatants into the best order
struct CombatantSorter{
    bool operator()(Combatant* a, Combatant* b){
        return a->getShipType() > b->getShipType();
    }
};

RSPCombat::RSPCombat() : objectcache(){
  obT_Fleet = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Fleet");
  obT_Planet = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Planet");
}

RSPCombat::~RSPCombat(){
}


void RSPCombat::doCombat(std::map<uint32_t, IdSet> sides){
    Game* game = Game::getGame();
    PlayerManager::Ptr playermanager = game->getPlayerManager();
    ObjectManager* objectmanager = game->getObjectManager();
    DesignStore::Ptr ds = game->getDesignStore();
    
    const char * const rsp[] = {"rock", "scissors", "paper"};
    
    IdSet listallobids;
    IdSet listallplayerids;
    
    std::map<uint32_t, std::vector<Combatant*> > fleetcache;
    
    battlelogger.reset(new BattleXML::BattleLogger());
    msgstrings.clear();
    
    for(std::map<uint32_t, IdSet>::iterator itmap = sides.begin(); itmap != sides.end(); ++itmap){
        std::vector<Combatant*> pcombatant;
        Player::Ptr player = playermanager->getPlayer(itmap->first);
        battlelogger->startSide(player->getName());
        IdSet theset = itmap->second;
        for(IdSet::iterator itset = theset.begin(); itset != theset.end(); ++itset){
            listallobids.insert(*itset);
            IGObject::Ptr obj = objectmanager->getObject (*itset);
            objectcache[*itset] = obj;
            if(obj->getType() == obT_Fleet){
                Fleet* f2 = (Fleet*)(obj->getObjectBehaviour());
                IdMap shiplist = f2->getShips();
                uint32_t damage = f2->getDamage();
                for(IdMap::reverse_iterator itship = shiplist.rbegin(); itship != shiplist.rend(); ++itship){
                    for(uint32_t i = 0; i < itship->second; i++){
                        Combatant* f1 = new Combatant();
                        f1->setOwner(itmap->first);
                        f1->setObject(obj->getID());
                        f1->setShipType(itship->first);
                        uint32_t mydamage = damage / std::max(1U, (unsigned int)(shiplist.size() - i));
                        f1->setDamage(mydamage);
                        damage -= mydamage;
                        std::string type = ds->getDesign(itship->first)->getName();
                        f1->setBattleXmlType(type);
                        f1->setBattleXmlId(str(boost::format("%1%-%2%-%3%") % type % obj->getID() % i));
                        f1->setBattleXmlName(str(boost::format("%1%'s %2%, %3% %4%") % player->getName() % obj->getName() % type % i));
                        battlelogger->addCombatant(f1);
                        pcombatant.push_back(f1);
                    }
                }
            }else{
                int shipcount = 2;
                int homeplanetid = game->getResourceManager()->getResourceDescription("Home Planet")->getResourceType();
                if(((Planet*)(obj->getObjectBehaviour()))->getResource(homeplanetid) == 1){
                    //three more for home planets
                    shipcount += 3;
                }
                for(int i = 0; i < shipcount; i++){
                    Combatant* f1 = new Combatant();
                    f1->setOwner(itmap->first);
                    f1->setObject(obj->getID());
                    f1->setShipType(0);
                    f1->setBattleXmlType("planet");
                    f1->setBattleXmlId(str(boost::format("planet-%1%-%2%") % obj->getID() % i));
                    f1->setBattleXmlName(str(boost::format("%1%'s colony on %2%, Defense battery %3%") % player->getName() % obj->getName() % i));
                    battlelogger->addCombatant(f1);
                    pcombatant.push_back(f1);
                }
            }
        }
        listallplayerids.insert(itmap->first);
        battlelogger->endSide();
        //sort combatant list by ship type, descending
        sort(pcombatant.begin(), pcombatant.end(), CombatantSorter());
        fleetcache[itmap->first] = pcombatant;
    }

    for(std::set<uint32_t>::iterator itplayer = listallplayerids.begin(); itplayer != listallplayerids.end(); ++itplayer){
        msgstrings[*itplayer] = "";
    }
  
  

    Random* random = Game::getGame()->getRandom();
    
    Logger::getLogger()->debug("Combat start");
    
    while(fleetcache.size() >= 2){
        battlelogger->startRound();
        uint32_t pos1, pos2;
        if(fleetcache.size() == 2){
            pos1 = 0;
            pos2 = 1;
        }else{
            pos1 = pos2 = random->getInRange(0U, ((uint32_t)(fleetcache.size() - 1)));
            while(pos2 == pos1){
                pos2 = random->getInRange(0U, ((uint32_t)(fleetcache.size() - 1)));
            }
        }
        
        std::map<uint32_t, std::vector<Combatant*> >::iterator itpa = fleetcache.begin();
        advance(itpa, pos1);
        std::map<uint32_t, std::vector<Combatant*> >::iterator itpb = fleetcache.begin();
        advance(itpb, pos2);
        
        std::vector<Combatant*> f1 = itpa->second;
        std::vector<Combatant*> f2 = itpb->second;
        
        uint32_t ownerid1 = f1[0]->getOwner();
        uint32_t ownerid2 = f2[0]->getOwner();
        
        std::string p1name = playermanager->getPlayer(ownerid1)->getName();
        std::string p2name = playermanager->getPlayer(ownerid2)->getName();
            
        int32_t r1 = random->getInRange((int32_t)0, (int32_t)2);
        int32_t r2 = random->getInRange((int32_t)0, (int32_t)2);
        
        std::map<Combatant*, uint32_t> d1, d2;
        
        battlelogger->log(str(boost::format("%1%'s fleet chooses %2%.") % p1name % rsp[r2]));
        battlelogger->log(str(boost::format("%1%'s fleet chooses %2%.") % p2name % rsp[(r2 + r1) % 3]));
        
        if(r1 == 0){
            //draw
            battlelogger->log("It's a draw");
            d1 = buildShotList(f1, true);
            d2 = buildShotList(f2, true);
            
            if(d1.size() != 0)
                doDamage(d1, f2);
            if(d2.size() != 0)
                doDamage(d2, f1);
        }else{
            
            if(r1 == 1){
                //pa win
                battlelogger->log(str(boost::format("%1% wins.") % p1name));
            
                d1 = buildShotList(f1);
                if(d1.size() == 0){
                    battlelogger->log(str(boost::format("%1%'s forces escape") % p1name));
                    msgstrings[ownerid1] += "Your Fleet escaped. ";
                    for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
                            msgit != msgstrings.end(); ++msgit){
                        if(msgit->first == ownerid1)
                            continue;
                        msgit->second += str(boost::format("%1%'s fleet of escaped. ") % p1name);
                    }
                    resolveCombatantsToObjects(f1);
                    for(std::vector<Combatant*>::iterator itcombatant = f1.begin(); itcombatant != f1.end();
                            ++itcombatant){
                        delete *itcombatant;
                    }
                    fleetcache.erase(itpa);
                    battlelogger->endRound();
                    continue;
                }
                doDamage(d1, f2);
            }else{
                //pb win
                battlelogger->log(str(boost::format("%1% wins.") % p2name));
                d2 = buildShotList(f2);
                if(d2.size() == 0){
                    battlelogger->log(str(boost::format("%1%'s forces escape") % p2name));
                    msgstrings[ownerid2] += "Your Fleet escaped. ";
                    for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
                            msgit != msgstrings.end(); ++msgit){
                        if(msgit->first == ownerid2)
                            continue;
                        msgit->second += str(boost::format("%1%'s fleet of escaped. ") % p2name);
                    }
                    resolveCombatantsToObjects(f2);
                    for(std::vector<Combatant*>::iterator itcombatant = f2.begin(); itcombatant != f2.end();
                            ++itcombatant){
                        delete *itcombatant;
                    }
                    fleetcache.erase(itpb);
                    battlelogger->endRound();
                    continue;
                }
                doDamage(d2, f1);
            }
            
        }
        
        if(isAllDead(f1)){
            msgstrings[ownerid1] += str(boost::format("Your fleet was destroyed by %1%'s fleet. ") % p2name);
            msgstrings[ownerid2] += str(boost::format("You destroyed %1%'s fleet. ") % p1name);
            std::string deathmsg = str(boost::format("%1%'s fleet destroyed %2%'s fleet. ") % p1name % p2name);
            for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
                    msgit != msgstrings.end(); ++msgit){
                if(msgit->first == ownerid1 || msgit->first == ownerid2)
                    continue;
                msgit->second += deathmsg;
            }
            resolveCombatantsToObjects(f1);
            for(std::vector<Combatant*>::iterator itcombatant = f1.begin(); itcombatant != f1.end();
                    ++itcombatant){
                delete *itcombatant;
            }
            fleetcache.erase(itpa);
        }
        if(isAllDead(f2)){
            msgstrings[ownerid2] += str(boost::format("Your fleet was destroyed by %1%'s fleet. ") % p1name);
            msgstrings[ownerid1] += str(boost::format("You destroyed %1%'s fleet. ") % p2name);
            std::string deathmsg = str(boost::format("%1%'s fleet destroyed %2%'s fleet. ") % p2name % p1name);
            for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
                    msgit != msgstrings.end(); ++msgit){
                if(msgit->first == ownerid1 || msgit->first == ownerid2)
                    continue;
                msgit->second += deathmsg;
            }
            resolveCombatantsToObjects(f2);
            for(std::vector<Combatant*>::iterator itcombatant = f2.begin(); itcombatant != f2.end();
                    ++itcombatant){
                delete *itcombatant;
            }
            fleetcache.erase(itpb);
        }
    
        battlelogger->endRound();
    
    }
    
    std::string file = battlelogger->save();
    
    if(!fleetcache.empty()){
      std::vector<Combatant*> flast = fleetcache.begin()->second;
      resolveCombatantsToObjects(flast);
      msgstrings[flast[0]->getOwner()] += "Your Fleet survived combat.";
      for(std::vector<Combatant*>::iterator itcombatant = flast.begin(); itcombatant != flast.end();
              ++itcombatant){
          delete *itcombatant;
      }
      fleetcache.erase(fleetcache.begin());
      if(!fleetcache.empty()){
          Logger::getLogger()->warning("fleetcache not empty at end of combat");
      }
    }
    for(std::map<uint32_t, std::string>::iterator msgit = msgstrings.begin(); 
        msgit != msgstrings.end(); ++msgit){
        Message::Ptr msg( new Message() );
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
    
    for(std::map<uint32_t, IGObject::Ptr>::iterator itob = objectcache.begin(); 
        itob != objectcache.end(); ++itob){
        Game::getGame()->getObjectManager()->doneWithObject(itob->first);
    }
    objectcache.clear();
  
}

std::map<Combatant*, uint32_t> RSPCombat::buildShotList(std::vector<Combatant*> combatants, bool isDraw){
    std::map<Combatant*, uint32_t> shotlist;
    uint32_t scoutcount = 0;
    designid_t biggestaliveshiptype = UINT32_NEG_ONE;
    for(std::vector<Combatant*>::iterator itc = combatants.begin(); itc != combatants.end();
            ++itc){
        if(!(*itc)->isDead()){
            if(biggestaliveshiptype == UINT32_NEG_ONE){
                biggestaliveshiptype = (*itc)->getShipType();
            }
            uint32_t shot = (*itc)->firepower(isDraw);
            if(!isDraw && shot == 0){
                scoutcount++;
            }
            if(!(isDraw && shot == 0) && (*itc)->getShipType() == biggestaliveshiptype){
                shotlist[*itc] = shot;
            }
        }
    }
    if(!isDraw){
        //check if scouts allow escape
        if(scoutcount == combatants.size()){
            shotlist.clear();
        }else if(scoutcount != 0){
            if(Game::getGame()->getRandom()->getReal1() < (double)scoutcount / (double)(combatants.size())){
                shotlist.clear();
            }
        }
    }
    return shotlist;
}

void RSPCombat::doDamage(std::map<Combatant*, uint32_t> shotlist, std::vector<Combatant*> targets){
    for(std::map<Combatant*, uint32_t>::iterator itshot = shotlist.begin(); itshot != shotlist.end();
            ++itshot){
        std::vector<Combatant*> possibletarget;
        for(std::vector<Combatant*>::iterator ittarget = targets.begin(); ittarget != targets.end();
                ++ittarget){
            if(!(*ittarget)->isDead()){
                if(possibletarget.size() == 0 || (*ittarget)->getShipType() == possibletarget[0]->getShipType()){
                    possibletarget.push_back(*ittarget);
                }
            }
            if(possibletarget.size() != 0 && (*ittarget)->getShipType() != possibletarget[0]->getShipType()){
                break;
            }
        }
        
        if(possibletarget.size() == 0){
            //all dead
            return;
        }
        
        uint32_t targetidx = 0;
        //find least damaged to shoot
        for(uint32_t i = 1; i < possibletarget.size(); i++){
            if(possibletarget[i]->getDamage() < possibletarget[targetidx]->getDamage()){
                targetidx = i;
            }
        }
        
        battlelogger->fire(itshot->first, possibletarget[targetidx]);
        
        battlelogger->damage(possibletarget[targetidx], itshot->second);
        
        if(possibletarget[targetidx]->hit(itshot->second)){
            battlelogger->death(possibletarget[targetidx]);
        }
        
        
    }
}

bool RSPCombat::isAllDead(std::vector<Combatant*> combatants){
    for(std::vector<Combatant*>::iterator itcurr = combatants.begin(); itcurr != combatants.end(); ++itcurr){
        if(!(*itcurr)->isDead()) return false;
    }
    return true;
}

void RSPCombat::resolveCombatantsToObjects(std::vector<Combatant*> combatants){
    
    std::map<objectid_t, bool> colonydead;
    
    std::map<objectid_t, uint32_t> damagemap;
    
    for(std::vector<Combatant*>::iterator itcombat = combatants.begin(); itcombat != combatants.end(); ++itcombat){
        Combatant* combatant = *itcombat;
        if(combatant->getShipType() == 0){
            //special for planets
            std::map<objectid_t, bool>::iterator itplanet = colonydead.find(combatant->getObject());
            if(itplanet == colonydead.end()){
                colonydead[combatant->getObject()] = combatant->isDead();
            }else{
                itplanet->second &= combatant->isDead();
            }
        }else if(combatant->isDead()){
            IGObject::Ptr obj = objectcache[combatant->getObject()];
            Fleet* fleet = dynamic_cast<Fleet*>(obj->getObjectBehaviour());
            if(fleet != NULL){
                fleet->removeShips(combatant->getShipType(), 1);
                if(fleet->totalShips() == 0){
                    //the fleet dead
                    //message player
                    msgstrings[combatant->getOwner()] += str(boost::format("Your fleet %1% was destroyed. ") % obj->getName());
                    //remove object
                    uint32_t queueid = static_cast<OrderQueueObjectParam*>(obj->getParameterByType(obpT_Order_Queue))->getQueueId();
                    OrderQueue::Ptr queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
                    queue->removeOwner(combatant->getOwner());
                    queue->removeAllOrders();
                    Game::getGame()->getObjectManager()->scheduleRemoveObject(obj->getID());
                    Game::getGame()->getPlayerManager()->getPlayer(fleet->getOwner())->getPlayerView()->removeOwnedObject(obj->getID());
                }
            }
        }else{
            if(damagemap.find(combatant->getObject()) == damagemap.end()){
                damagemap[combatant->getObject()] = combatant->getDamage();
            }else{
                damagemap[combatant->getObject()] += combatant->getDamage();
            }
        }
    }
    
    for(std::map<objectid_t, uint32_t>::iterator itobj = damagemap.begin(); itobj != damagemap.end(); ++itobj){
        IGObject::Ptr obj = objectcache[itobj->first];
        Fleet* fleet = dynamic_cast<Fleet*>(obj->getObjectBehaviour());
        if(fleet == NULL){
            warningLog("Fleet was not a fleet");
            continue;
        }
        fleet->setDamage(itobj->second);
    }

    for(std::map<objectid_t, bool>::iterator itplanet = colonydead.begin(); itplanet != colonydead.end(); ++itplanet){
        if(itplanet->second){
            IGObject::Ptr obj = objectcache[itplanet->first];
            Planet* planet = dynamic_cast<Planet*>(obj->getObjectBehaviour());
            if(planet == NULL){
                warningLog("Planet was not a planet");
                continue;
            }
            resourcetypeid_t homeplanetres = Game::getGame()->getResourceManager()->getResourceDescription("Home Planet")->getResourceType();
            bool ishomeplanet = (planet->getResource(homeplanetres) == 1);
            
            //planet has fallen
            if(ishomeplanet){
                planet->removeResource(homeplanetres, 1);
            }
            uint32_t oldowner = planet->getOwner();
            planet->setOwner(0);
            uint32_t queueid = static_cast<OrderQueueObjectParam*>(obj->getParameterByType(obpT_Order_Queue))->getQueueId();
            OrderQueue::Ptr queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
            queue->removeOwner(oldowner);
            queue->removeAllOrders();
            Game::getGame()->getPlayerManager()->getPlayer(oldowner)->getPlayerView()->removeOwnedObject(obj->getID());
        }
    }
    
}

