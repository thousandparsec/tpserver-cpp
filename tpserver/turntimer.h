#ifndef TURNTIMER_H
#define TURNTIMER_H
/*  TurnTimer class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#include <string>

class TimerCallback;

class TurnTimer{
  public:
    TurnTimer();
    virtual ~TurnTimer();
    
    void playerFinishedTurn(uint32_t playerid);
    uint32_t secondsToEOT() const;
    
    virtual void resetTimer();
    
    void manuallyRunEndOfTurn();
    
  private:
    void updateTimer();
    void timerFinished();
    void thresholdDoneAndStartEOT();
    bool isOverThreshold();
    void thresholdChanged(const std::string& key, const std::string& val);
    
    TimerCallback* timer;
    std::set<uint32_t> finishedPlayers;
    bool overthreshold;
};

#endif
