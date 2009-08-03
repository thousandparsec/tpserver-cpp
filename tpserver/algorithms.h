#ifndef ALGORITHMS_H
#define ALGORITHMS_H

/*  Common STL/Boost based algorithms
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

#include <functional>

// Structure for selecting first element of a pair
template <typename pair_type>
struct select1st: public std::unary_function<pair_type, typename pair_type::first_type>
{
  const typename pair_type::first_type operator()(const pair_type& v) const
  {
    return v.first;
  }
};

// Structure for selecting second element of a pair
template <typename pair_type>
struct select2nd: public std::unary_function<pair_type, typename pair_type::second_type>
{
  const typename pair_type::second_type operator()(const pair_type& v) const
  {
    return v.second;
  }
};

// Mass delete algorithm
// TODO: remove once the server is converted to all shared pointers
template <typename container>
void delete_all( container& cont )
{
  for ( typename container::iterator iter = cont.begin(); iter != cont.end(); ++iter )
    delete *iter;
}

#endif // ALGORITHMS_H
