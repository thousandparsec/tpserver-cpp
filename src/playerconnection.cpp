/*  Player Connection object
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"
#include "playermanager.h"
#include "settings.h"

#include "playerconnection.h"

PlayerConnection::PlayerConnection() : Connection()
{
	player = NULL;
}


PlayerConnection::PlayerConnection(int fd) : Connection()
{
	sockfd = fd;
	status = 1;
	player = NULL;
}

PlayerConnection::~PlayerConnection()
{

}


void PlayerConnection::setFD(int fd)
{
	sockfd = fd;
	status = 1;

}

void PlayerConnection::process()
{
	Logger::getLogger()->debug("About to Process");
	switch (status) {
	case 1:
		//check if user is really a TP protocol verNN client
		Logger::getLogger()->debug("Stage1 : pre-connect");
		verCheck();
		break;
	case 2:
		//authorise the user
		Logger::getLogger()->debug("Stage2 : connected");
		login();
		break;
	case 3:
		//process as normal
		Logger::getLogger()->debug("Stage3 : logged in");
		inGameFrame();
		break;
	case 0:
	default:
		//do nothing
		Logger::getLogger()->warning("Tried to process connections that is closed or invalid");
		if (status != 0)
			close();
		status = 0;
		break;
	}
	Logger::getLogger()->debug("Finished Processing");
}


Frame* PlayerConnection::createFrame(Frame* oldframe)
{
  Frame* newframe;
  if(oldframe != NULL) {
    newframe = new Frame(oldframe->getVersion());
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe = new Frame(version);
    newframe->setSequence(0);
  }
  return newframe;
}


void PlayerConnection::login()
{
    Frame *recvframe = createFrame();
    if (readFrame(recvframe)) {
        if(recvframe->getType() == ft02_Login){
            char *username = recvframe->unpackString();
            char *password = recvframe->unpackString();
            if (username != NULL && password != NULL) {
                //authenicate
                try{
                    player = Game::getGame()->getPlayerManager()->findPlayer(username, password);
                }catch(std::exception e){
                    if (Settings::getSettings()->get("autoadd_players") == "yes") {
                        Logger::getLogger()->info("Creating new player automatically");
                        player = Game::getGame()->getPlayerManager()->createNewPlayer(username, password);
                    }
                }
                if(player != NULL){
				Frame *okframe = createFrame(recvframe);
				okframe->setType(ft02_OK);
				okframe->packString("Welcome");
				sendFrame(okframe);
				Logger::getLogger()->info("Login OK!");
				player->setConnection(this);
				status = 3;
			} else {
                            Logger::getLogger()->info("Bad username or password");
				Frame *failframe = createFrame(recvframe);
				failframe->createFailFrame(fec_FrameError, "Login Error - bad username or password");	// TODO - should be a const or enum, Login error
				sendFrame(failframe);
			}
		} else {
			Logger::getLogger()->debug("username or password == NULL");
			Frame *failframe = createFrame(recvframe);
			failframe->createFailFrame(fec_FrameError, "Login Error - no username or password");	// TODO - should be a const or enum, Login error
			sendFrame(failframe);
			close();
		}
		if (username != NULL)
			delete[] username;
		if (password != NULL)
			delete[] password;

	
	  }else if(version >= fv0_3 && recvframe->getType() == ft03_Features_Get){
	    Logger::getLogger()->debug("Processing Get Features frame");
	    Frame *features = createFrame(recvframe);
	    Network::getNetwork()->createFeaturesFrame(features);
	    sendFrame(features);
            }else if(recvframe->getType() == ft02_Time_Remaining_Get){
                Logger::getLogger()->debug("Processing Get Time frame");
                Frame *time = createFrame(recvframe);
                time->setType(ft02_Time_Remaining);
                time->packInt(Game::getGame()->secondsToEOT());
                sendFrame(time);
            }else{
                Logger::getLogger()->warning("In connected state but did not receive login, get features or get get time remaining");
	    Frame *failframe = createFrame(recvframe);
	    failframe->createFailFrame(fec_FrameError, "Wrong type of frame in this state, wanted login or get features");
	    sendFrame(failframe);
	  }
	}
	delete recvframe;
	  
}

void PlayerConnection::inGameFrame()
{
	Frame *frame = createFrame();
	if (readFrame(frame)) {
		
		if(version >= fv0_3 && frame->getType() == ft03_Features_Get){
		  Logger::getLogger()->debug("Processing Get Features frame");
		  Frame *features = createFrame(frame);
		  Network::getNetwork()->createFeaturesFrame(features);
		  sendFrame(features);
		}else if(version >= fv0_3 && frame->getType() == ft03_Ping){
		  Logger::getLogger()->debug("Processing Ping frame");
		  // check for the time of the last frame, ignore this if
		  //  less than 60 seconds ago.
		  //TODO last frame check
		  Frame *pong = createFrame(frame);
		  pong->setType(ft02_OK);
		  pong->packString("Keep alive ok, hope you're still there");
		  sendFrame(pong);
                }else if(frame->getType() == ft02_Time_Remaining_Get){
                    Logger::getLogger()->debug("Processing Get Time frame");
                    Frame *time = createFrame(frame);
                    time->setType(ft02_Time_Remaining);
                    time->packInt(Game::getGame()->secondsToEOT());
                    sendFrame(time);
		}else{
		  if(Game::getGame()->isStarted()){
		    // should pass frame to player to do something with
		    Logger::getLogger()->debug("inGameFrame");
		    player->processIGFrame(frame);
		  }else{
		    Frame *gns = createFrame(frame);
		    gns->createFailFrame(fec_TempUnavailable, "Game has not yet started, please wait");
		    sendFrame(gns);
		    delete frame;
		  }
		}
	} else {
		Logger::getLogger()->debug("noFrame :(");
		// client closed
		delete frame;
	}
}

