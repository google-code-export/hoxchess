#ifndef INT_H
#define INT_H

#include <boost/cstdint.hpp>

namespace folium
{
    typedef boost::int8_t sint8;
    typedef boost::uint8_t uint8;
    typedef boost::int16_t sint16;
    typedef boost::uint16_t uint16;
    typedef boost::int32_t sint32;
    typedef boost::uint32_t uint32;
    typedef boost::int64_t sint64;
    typedef boost::uint64_t uint64;

    typedef boost::int_fast32_t sint;
    typedef boost::uint_fast32_t uint;
}//namespace folium

#endif //INT_H
