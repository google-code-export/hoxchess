#ifndef _XQ_DATA_H_
#define _XQ_DATA_H_

#include <cstring>
#include "defines.h"
namespace folium
{
    extern const uint8 g_xs[91];
    extern const uint8 g_ys[91];
    extern const uint8 g_xys[16][16];
    extern const uint8 g_coordinate_downs[91];
    extern const uint8 g_coordinate_ups[91];
    extern const uint8 g_coordinate_lefts[91];
    extern const uint8 g_coordinate_rights[91];
    extern const uint16 g_coordinate_flags[91];

    extern const uint8 g_piece_types[34];
    extern const uint16 g_piece_flags[34];
    extern const uint16 g_piece_colors[34];

    extern const uint8 g_simple_values[32];

    extern const uint8 g_type_begin_index[16];
    extern const uint8 g_type_end_index[16];

    extern const uint16 g_move_flags[91][128];

    extern const uint8 g_knight_legs[91][128];

    inline uint coordinate_x(uint sq)
    {
        assert (sq < 91UL);
        return g_xs[sq];
    }
    inline uint coordinate_y(uint sq)
    {
        assert (sq < 91UL);
        return g_ys[sq];
    }
    inline uint xy_coordinate(uint x, uint y)
    {
        assert (x < 16UL);
        assert (y < 16UL);
        return g_xys[y][x];
    }
    inline uint coordinate_down(uint sq)
    {
        assert (sq < 91UL);
        return g_coordinate_downs[sq];
    }
    inline uint coordinate_up(uint sq)
    {
        assert (sq < 91UL);
        return g_coordinate_ups[sq];
    }
    inline uint coordinate_left(uint sq)
    {
        assert (sq < 91UL);
        return g_coordinate_lefts[sq];
    }
    inline uint coordinate_right(uint sq)
    {
        assert (sq < 91UL);
        return g_coordinate_rights[sq];
    }
    inline uint coordinate_flag(uint sq)
    {
        assert (sq < 91UL);
        return g_coordinate_flags[sq];
    }

    inline uint piece_type(uint piece)
    {
        assert (piece < 34UL);
        return g_piece_types[piece];
    }

    inline uint piece_flag(uint piece)
    {
        assert (piece < 34UL);
        return g_piece_flags[piece];
    }
    inline uint piece_color(uint piece)
    {
        assert (piece < 34UL);
        return g_piece_colors[piece];
    }

    inline uint type_flag(uint type)
    {
        assert (type < 16UL);
        return 1UL << type;
    }

    inline uint type_begin(uint type)
    {
        assert (type < 16UL);
        return g_type_begin_index[type];
    }
    inline uint type_end(uint type)
    {
        assert (type < 16UL);
        return g_type_end_index[type];
    }

    inline uint knight_leg(uint src, uint dst)
    {
        assert (src < 91UL);
        assert (dst < 91UL);
        return g_knight_legs[src][dst];
    }

    inline uint bishop_eye(uint src, uint dst)
    {
        assert (src < 91UL);
        assert (dst < 91UL);
        assert (g_move_flags[dst][src] & BishopFlag);
        return (src+dst)/2;
    }

    extern const uint16 g_kinght_moves[91][16];
    extern const uint8 g_red_king_pawn_moves[91][8];
    extern const uint8 g_black_king_pawn_moves[91][8];
    extern const uint8 g_advisor_bishop_moves[91][8];

}//namespace folium

#endif //_XQ_DATA_H_
