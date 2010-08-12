#ifndef ORDERPARAMETERS_H
#define ORDERPARAMETERS_H
/*  Order Parameter classes
 *
 *  Copyright (C) 2009 Kornel Kisielewicz and the Thousand Parsec Project
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

#include <tpserver/orderparameter.h>
#include <tpserver/common.h>
#include <tpserver/vector3d.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class ListParameter : public OrderParameter{

  public:
    typedef std::pair<std::string, uint32_t> Option;
    typedef std::map< uint32_t, Option> Options;
    typedef boost::function< Options () > Callback;

    ListParameter(const std::string& aname, const std::string& adesc, Callback acallback);

    virtual void pack(OutputFrame::Ptr f) const;
    virtual bool unpack( InputFrame::Ptr f);

    IdMap getList() const { return list; };
    void setList(IdMap nlist) { list = nlist; }; 

  protected:
    IdMap list;
    Callback callback;
};

class TimeParameter : public OrderParameter{

  public:
    TimeParameter( const std::string& aname, const std::string& adesc, uint32_t time = 0 );

    virtual void pack(OutputFrame::Ptr f) const;
    virtual bool unpack( InputFrame::Ptr f);

    uint32_t getTime() const { return turns; }
    void setTime(uint32_t time) { turns = time; }

  protected:
    uint32_t turns;

};

class StringParameter : public OrderParameter{

  public:
    StringParameter( const std::string& aname, const std::string& adesc );

    virtual void pack(OutputFrame::Ptr f) const;
    virtual bool unpack( InputFrame::Ptr f);

    std::string getString() const { return string; };
    void setString(const std::string& rhs) { string = rhs; };

  protected:
    std::string string;

};

class SpaceCoordParam : public OrderParameter{

  public:
    SpaceCoordParam( const std::string& aname, const std::string adesc );

    virtual void pack(OutputFrame::Ptr f) const;
    virtual bool unpack( InputFrame::Ptr f);

    Vector3d getPosition() const { return position; }
    void setPosition(const Vector3d& pos) { position = pos; } 

  protected:
    Vector3d position;

};

class ObjectOrderParameter : public OrderParameter{

  public:
    ObjectOrderParameter( const std::string& aname, const std::string& adesc, IdSet objecttypes = IdSet());

    virtual void pack(OutputFrame::Ptr f) const;
    virtual bool unpack( InputFrame::Ptr f);

    uint32_t getObjectId() const;
    void setObjectId(uint32_t id);

  protected:
    uint32_t object;
    IdSet allowedtypes;

};

#endif
