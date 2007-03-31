#ifndef ORDER_H
#define ORDER_H
/*  Order base class
 *
 *  Copyright (C) 2004-2005,2007  Lee Begg and the Thousand Parsec Project
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
#include <list>
#include <map>
#include <string>

#include "result.h"

class Frame;
class IGObject;
class OrderParameter;

class Order {

      public:
	Order();
        virtual ~Order();

	int getType() const;
	void setType(int ntype);
        std::string getName() const;
        uint32_t getTurns() const;
        void setTurns(uint32_t nturns);
        std::map<uint32_t, uint32_t> getResources() const;
        void addResource(uint32_t resid, uint32_t amount);
        std::list<OrderParameter*> getParameters() const;
        uint64_t getDescriptionModTime() const;

	virtual void createFrame(Frame * f, int objID, int pos);
	virtual Result inputFrame(Frame * f, unsigned int playerid);

	virtual bool doOrder(IGObject * ob) = 0;

	void describeOrder(Frame * f) const;
	virtual Order *clone() const = 0;

      protected:
	 uint32_t type;
	 uint64_t descmodtime;
         std::string name;
         std::string description;
         uint32_t turns;
         std::map<uint32_t, uint32_t> resources;
         std::list<OrderParameter*> parameters;

};

#endif
