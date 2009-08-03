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
#include <algorithm>
#include <set>

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
template <typename container_type >
void delete_all( container_type& cont )
{
  for ( typename container_type::iterator iter = cont.begin(); iter != cont.end(); ++iter )
    delete *iter;
}

// Mass delete algorithm
// TODO: remove once the server is converted to all shared pointers
template <typename container_type >
void delete_map_all( container_type& cont )
{
  for ( typename container_type::iterator iter = cont.begin(); iter != cont.end(); ++iter )
    delete iter->second;
}

// Fills map-like container with values based on given set
template < typename container_type >
void fill_by_set( container_type& cont, 
                  const std::set< typename container_type::key_type >& set, 
                  typename container_type::mapped_type value )
{
  typedef std::set< typename container_type::key_type > key_set;
  for ( typename key_set::const_iterator iter = set.begin(); iter != set.end(); ++iter )
    cont[ *iter ] = value;
}

// Fills a set with keys from a container with values based on given set
// Would be more effective if doing it in-place.
template < typename container_type >
std::set< typename container_type::key_type > generate_key_set( const container_type& cont )
{
  typedef std::set< typename container_type::key_type > key_set;
  key_set set;
  std::transform( cont.begin(), cont.end(), std::inserter< key_set >( set, set.begin() ), select1st< typename container_type::value_type >() );
  return set;
}

// Looks for an element in a map, if found, returns it, if not returns default value
template < typename container_type >
typename container_type::mapped_type find_default( const container_type& cont, typename container_type::key_type key, typename container_type::mapped_type value )
{
  typename container_type::const_iterator iter = cont.find( key );
  if ( iter != cont.end() )
    return iter->second;
  else
    return value;
}

// Runs function object on every item in range that satisfies predicate
template < typename iterator, typename predicate, typename function >
void for_each_if( iterator first, iterator last, predicate pred, function func )
{
  for ( ; first != last; ++first )
    if ( pred( *first ) )
        func( *first );
}

// Runs function object on every map key in range 
template < typename iterator, typename function >
void for_each_key( iterator first, iterator last, function func )
{
  for ( ; first != last; ++first )
    func( first->first );
}

// Runs function object on every map value in range 
template < typename iterator, typename function >
void for_each_value( iterator first, iterator last, function func )
{
  for ( ; first != last; ++first )
    func( first->second );
}

#endif // ALGORITHMS_H
