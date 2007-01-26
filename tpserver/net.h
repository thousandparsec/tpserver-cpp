#ifndef NET_H
#define NET_H
/*  Network Abstraction class
 *
 *  Copyright (C) 2004-2005, 2006, 2007  Lee Begg and the Thousand Parsec Project
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

class Connection;
class Frame;
class Advertiser;

class TimerCallback;

enum FeatureIDs {
  fid_sec_conn_this = 1,
  fid_sec_conn_other = 2,
  fid_http_this = 3,
  fid_http_other = 4,
  fid_keep_alive = 5,
  fid_serverside_property = 6,
  fid_account_register = 1000
};

class Network {

      public:
	static Network *getNetwork();

	// Feature frames
	void createFeaturesFrame(Frame * frame);
        void addFeature(int featid, int value);
        void removeFeature(int featid);

	//stuff

	void addConnection(Connection* conn);
	void removeConnection(Connection* conn);
        void addToWriteQueue(Connection* conn);
        
        void addTimer(TimerCallback callback);
        bool removeTimer(TimerCallback callback);

	void start();

	void stop();
        
        bool isStarted() const;

	void sendToAll(Frame * frame);

	// don't you even think about calling these functions

	void masterLoop();
	void stopMainLoop();
        

      private:
	 Network();
	~Network();
	 Network(Network & rhs);
	Network operator=(Network & rhs);

        void addAccountSettingChanged(const std::string &item, const std::string &value);

	static Network *myInstance;

	fd_set master_set;
	int max_fd;
	bool active;

	//pthread_t master;


	bool halt;

	 std::map < int, Connection * >connections;
         std::map<int, Connection*> writequeue;
         
         std::priority_queue<TimerCallback, std::vector<TimerCallback>, std::greater<TimerCallback> > timers;

         std::map<int,int> features;
         
         Advertiser* advertiser;

};

#endif
