#ifndef NET_H
#define NET_H
/*  Network Abstraction class
 *
 *  Copyright (C) 2004-2005, 2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

//#include <pthread.h>
#include <map>
#include <queue>
#include <functional>
#include <algorithm>

class Connection;
class Frame;
class Advertiser;
class AsyncFrame;

class TimerCallback;

class Network {

  public:
    static Network *getNetwork();

    //stuff

    void addConnection(Connection* conn);
    void removeConnection(Connection* conn);
    void addToWriteQueue(Connection* conn);

    void addTimer(TimerCallback callback);
    bool removeTimer(TimerCallback callback);

    void start();

    void stop();

    bool isStarted() const;

    void adminStart();

    void adminStop();

    void sendToAll(AsyncFrame* aframe);
    void doneEOT();

    Advertiser* getAdvertiser() const;

    // don't you even think about calling these functions

    void masterLoop();
    void stopMainLoop();


  private:
    typedef std::map< int, Connection*> ConnMap;
    Network();
    ~Network();
    Network(Network & rhs);
    Network operator=(Network & rhs);

    static Network *myInstance;

    fd_set master_set;
    int max_fd;
    bool active;

    //pthread_t master;

    volatile bool halt;

    ConnMap connections;
    ConnMap writequeue;

    std::priority_queue<TimerCallback, std::vector<TimerCallback>, std::greater<TimerCallback> > timers;

    Advertiser* advertiser;

};

#endif
