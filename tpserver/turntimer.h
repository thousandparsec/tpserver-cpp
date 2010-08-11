#ifndef TURNTIMER_H
#define TURNTIMER_H
/*  TurnTimer class
 *
 *  Copyright (C) 2007, 2009 Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>
#include <set>
#include <tpserver/timercallback.h>

class TurnTimer{
  public:
    TurnTimer();
    virtual ~TurnTimer();

    virtual uint32_t secondsToEOT() const;
    virtual uint32_t getTurnLength() const;
    virtual std::set<playerid_t> getPlayers();

    virtual void resetTimer() = 0;

    void manuallyRunEndOfTurn();

    void playerFinishedTurn(uint32_t playerid);
    void setNumberDeadPlayers(uint32_t ndp);

    void timerFinished();
  protected:
    uint32_t getNumActivePlayers() const;
    uint32_t getNumDonePlayers() const;

    virtual void onPlayerFinishedTurn();

    //Triggers EOT
    void timerExpiredStartEOT();
    void allDoneStartEOT();

    //Triggers alerts
    void timerStarted();
    void thresholdFinishedNewTimer();
    void advancedWarningOfTimer();
    
    TimerCallback::Ptr timer;

  private:
    void sendTimeRemaining(uint32_t remaining, uint32_t reason);
    void doEndOfTurn();
    std::set<uint32_t> finishedPlayers;
    uint32_t numdead;
};

#endif
