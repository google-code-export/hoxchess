#ifndef _XQ_POSITION_DATA_H_
#define _XQ_POSITION_DATA_H_

#include <cstring>

namespace folium
{
    extern const uint64 * const g_type_locks[14];
    extern const uint32 * const g_type_keys[14];
    extern const sint32 * const g_type_values[14];
    extern const uint64 * const g_piece_locks[32];
    extern const uint32 * const g_piece_keys[32];
    extern const sint32 * const g_piece_values[32];

    inline const uint64& piece_lock(uint piece, uint coordinate)
    {
        assert (piece < 32UL);
        assert (coordinate < 91UL);
        return g_piece_locks[piece][coordinate];
    }
    inline const uint32& piece_key(uint piece, uint coordinate)
    {
        assert (piece < 32UL);
        assert (coordinate < 91UL);
        return g_piece_keys[piece][coordinate];
    }
    inline const sint32& piece_value(uint piece, uint coordinate)
    {
        assert (piece < 32UL);
        assert (coordinate < 91UL);
        return g_piece_values[piece][coordinate];
    }

    inline const uint64& type_lock(uint type, uint coordinate)
    {
        assert (type < 14UL);
        assert (coordinate < 91UL);
        return g_type_locks[type][coordinate];
    }
    inline const uint32& type_key(uint type, uint coordinate)
    {
        assert (type < 14UL);
        assert (coordinate < 91UL);
        return g_type_keys[type][coordinate];
    }
    inline const sint32& type_value(uint type, uint coordinate)
    {
        assert (type < 14UL);
        assert (coordinate < 91UL);
        return g_type_values[type][coordinate];
    }

}//namespace folium

#endif //_XQ_POSITION_DATA_H_
