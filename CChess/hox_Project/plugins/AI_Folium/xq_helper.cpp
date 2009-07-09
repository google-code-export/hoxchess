#include <vector>
#include "xq.h"
#include "utility/str.h"
namespace folium
{
    using namespace std;
    uint player_in_check(const XQ& xq, uint player)
    {
        if (player == Red)
        {
            uint kp = xq.piece(RedKingIndex);
            assert(kp != InvaildCoordinate);
            return ((xq.coordinate_flag(coordinate_up(kp))
                     | xq.coordinate_flag(coordinate_left(kp))
                     | xq.coordinate_flag(coordinate_right(kp)))
                    & BlackPawnFlag)//pawn
                   | ((xq.coordinate_flag(xq.nonempty_down_1(kp))
                       | xq.coordinate_flag(xq.nonempty_up_1(kp))
                       | xq.coordinate_flag(xq.nonempty_left_1(kp))
                       | xq.coordinate_flag(xq.nonempty_right_1(kp)))
                      & (BlackRookFlag | BlackKingFlag))//rook
                   | ((xq.coordinate_flag(xq.nonempty_down_2(kp))
                       | xq.coordinate_flag(xq.nonempty_up_2(kp))
                       | xq.coordinate_flag(xq.nonempty_left_2(kp))
                       | xq.coordinate_flag(xq.nonempty_right_2(kp)))
                      & BlackCannonFlag)//cannon
                   | ((xq.coordinate_flag(knight_leg(xq.piece(BlackKnightIndex1), kp))
                       | xq.coordinate_flag(knight_leg(xq.piece(BlackKnightIndex2), kp)))
                      & EmptyFlag);//knight
        }
        else
        {
            uint kp = xq.piece(BlackKingIndex);
            assert(kp != InvaildCoordinate);
            return ((xq.coordinate_flag(coordinate_down(kp))
                     | xq.coordinate_flag(coordinate_left(kp))
                     | xq.coordinate_flag(coordinate_right(kp)))
                    & RedPawnFlag)//pawn
                   | ((xq.coordinate_flag(xq.nonempty_down_1(kp))
                       | xq.coordinate_flag(xq.nonempty_up_1(kp))
                       | xq.coordinate_flag(xq.nonempty_left_1(kp))
                       | xq.coordinate_flag(xq.nonempty_right_1(kp)))
                      & (RedRookFlag | RedKingFlag))//rook
                   | ((xq.coordinate_flag(xq.nonempty_down_2(kp))
                       | xq.coordinate_flag(xq.nonempty_up_2(kp))
                       | xq.coordinate_flag(xq.nonempty_left_2(kp))
                       | xq.coordinate_flag(xq.nonempty_right_2(kp)))
                      & RedCannonFlag)//cannon
                   | ((xq.coordinate_flag(knight_leg(xq.piece(RedKnightIndex1), kp))
                       | xq.coordinate_flag(knight_leg(xq.piece(RedKnightIndex2), kp)))
                      & EmptyFlag);//knight
        }
    }

    uint status(const XQ& xq)
    {
        if (player_in_check(xq, xq.player()))
            return 1;
        return 0;
    }

    bool is_good_cap(const XQ& xq, uint move)
    {
        uint dst = move_dst(move);
        uint dst_piece = xq.coordinate(dst);
        if (dst_piece == EmptyIndex)
            return false;
        uint src_piece = xq.coordinate(move_src(move));
        if (g_simple_values[src_piece] < g_simple_values[dst_piece])
            return true;
        if (xq.player() == Red)
        {
            //king
            {
                uint src = xq.piece(BlackKingIndex);
                if (g_move_flags[dst][src] & BlackPawnFlag)
                    return false;
            }
            //advisor
            for (uint idx = BlackAdvisorIndex1; idx <= BlackAdvisorIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackAdvisorFlag)
                    return false;
            }
            //bishop
            for (uint idx = BlackBishopIndex1; idx <= BlackBishopIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackBishopFlag)
                    if (xq.coordinate_is_empty(bishop_eye(src, dst)))
                        return false;
            }
            //rook
            for (uint idx = BlackRookIndex1; idx <= BlackRookIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackRookFlag)
                {
                    switch (xq.distance(src, dst))
                    {
                    case 0:
                        return false;
                    case 1:
                        if (xq.distance(src, move_src(move)) == 0)
                            return false;
                    }
                }
            }
            //knight
            for (uint idx = BlackKnightIndex1; idx <= BlackKnightIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackKnightFlag)
                    if (xq.coordinate_is_empty(knight_leg(src, dst)))
                        return false;
            }
            //cannon
            for (uint idx = BlackCannonIndex1; idx <= BlackCannonIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackCannonFlag)
                {
                    switch (xq.distance(src, dst))
                    {
                    case 1:
                        if (xq.distance(src, move_src(move)) != 0)
                            return false;
                        break;
                    case 2:
                        if (xq.distance(src, move_src(move)) < 2)
                            return false;
                    }
                }
            }
            //pawn
            for (uint idx = BlackPawnIndex1; idx <= BlackPawnIndex5; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & BlackPawnFlag)
                    return false;
            }
        }
        else
        {
            //king
            {
                uint src = xq.piece(RedKingIndex);
                if (g_move_flags[dst][src] & RedPawnFlag)
                    return false;
            }
            //advisor
            for (uint idx = RedAdvisorIndex1; idx <= RedAdvisorIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedAdvisorFlag)
                    return false;
            }
            //bishop
            for (uint idx = RedBishopIndex1; idx <= RedBishopIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedBishopFlag)
                    if (xq.coordinate_is_empty(bishop_eye(src, dst)))
                        return false;
            }
            //rook
            for (uint idx = RedRookIndex1; idx <= RedRookIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedRookFlag)
                {
                    switch (xq.distance(src, dst))
                    {
                    case 0:
                        return false;
                    case 1:
                        if (xq.distance(src, move_src(move)) == 0)
                            return false;
                    }
                }
            }
            //knight
            for (uint idx = RedKnightIndex1; idx <= RedKnightIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedKnightFlag)
                    if (xq.coordinate_is_empty(knight_leg(src, dst)))
                        return false;
            }
            //cannon
            for (uint idx = RedCannonIndex1; idx <= RedCannonIndex2; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedCannonFlag)
                {
                    switch (xq.distance(src, dst))
                    {
                    case 1:
                        if (xq.distance(src, move_src(move)) != 0)
                            return false;
                        break;
                    case 2:
                        if (xq.distance(src, move_src(move)) < 2)
                            return false;
                    }
                }
            }
            //pawn
            for (uint idx = RedPawnIndex1; idx <= RedPawnIndex5; ++idx)
            {
                uint src = xq.piece(idx);
                if (g_move_flags[dst][src] & RedPawnFlag)
                    return false;
            }
        }
        return true;
    }
    bool is_legal_move(const XQ& xq, uint32 src, uint32 dst)
    {
        if (src > 90 || dst > 90)
            return false;
        uint32 src_piece = xq.coordinate(src);
        //这里同时排除了src == dst
        if (piece_color(src_piece) == xq.coordinate_color(dst))
            return false;
        if (!(g_move_flags[dst][src] & piece_flag(src_piece)))
            return false;
        switch (piece_type(src_piece))
        {
        case RedKing:
            if (g_move_flags[dst][src] & RedPawnFlag)
                return true;
            return (coordinate_flag(dst) & BlackKingFlag) && xq.distance(src, dst) == 0;
        case BlackKing:
            if (g_move_flags[dst][src] & BlackPawnFlag)
                return true;
            return (xq.coordinate_flag(dst) & RedKingFlag) && xq.distance(src, dst) == 0;
        case RedAdvisor:
        case BlackAdvisor:
            return true;
        case RedBishop:
        case BlackBishop:
            return xq.coordinate_is_empty(bishop_eye(src, dst));
        case RedRook:
        case BlackRook:
            return xq.distance_is_0(src, dst);
        case RedKnight:
        case BlackKnight:
            return xq.coordinate_is_empty(knight_leg(src, dst));
        case RedCannon:
        case BlackCannon:
            return (xq.distance(src, dst) + (xq.coordinate(dst) >> 5)) == 1;
        case RedPawn:
        case BlackPawn:
            return true;
        default:
            return false;
        }
    }

    string mirror4fen(const string& fen, uint mirror)
    {
        assert(mirror < 4);
        XQ xq;
        if (!xq.set_fen(fen))
            return string();
        string ret = xq.get_fen();
        if (mirror & 1)
        {
            vector<string> vec = split(ret, " ");
            vector<string> lines = split(vec[0], "/");
            for (vector<string>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
                *itr = reverse(*itr);
            vec[0] = join(lines, "/");
            ret = join(vec, " ");
        }
        if (mirror & 2)
        {
            vector<string> vec = split(ret, " ");
            vec[0] = swapcase(reverse(vec[0]));
            vec[1][0] = xq.player() == Red ? 'b' : 'r';
            ret = join(vec, " ");
        }
        return ret;
    }
}
