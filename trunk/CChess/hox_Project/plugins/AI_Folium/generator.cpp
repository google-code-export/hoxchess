#include "generator.h"

namespace folium
{
    void generate_moves(const XQ& xq, MoveList &ml, const History& history)
    {
        uint own = xq.player();
        uint opp = 1 - own;
        uint idx;
        uint src, dst;
        ml.clear();
        if (own == Red)
        {
            idx = RedKingIndex;
            //red king
            src = xq.piece(RedKingIndex);
            const uint8 *pm = g_red_king_pawn_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) != own)
                    ml.push(history.move(src, dst));
                dst = *pm++;
            }
            //red pawn
            for (uint i = RedPawnIndex1; i <= RedPawnIndex5; ++i)
            {
                src = xq.piece(i);
                if (src == InvaildCoordinate)
                    continue;
                pm = g_red_king_pawn_moves[src];
                dst = *pm++;
                while (dst != InvaildCoordinate)
                {
                    if (xq.coordinate_color(dst) != own)
                        ml.push(history.move(src, dst));
                    dst = *pm++;
                }
            }
        }
        else
        {
            idx = BlackKingIndex;
            //black king
            src = xq.piece(BlackKingIndex);
            const uint8 *pm = g_black_king_pawn_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) != own)
                    ml.push(history.move(src, dst));
                dst = *pm++;
            }
            //black pawn
            for (uint i = BlackPawnIndex1; i <= BlackPawnIndex5; ++i)
            {
                src = xq.piece(i);
                if (src == InvaildCoordinate)
                    continue;
                pm = g_black_king_pawn_moves[src];
                dst = *pm++;
                while (dst != InvaildCoordinate)
                {
                    if (xq.coordinate_color(dst) != own)
                        ml.push(history.move(src, dst));
                    dst = *pm++;
                }
            }
        }
        //advisor
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint8 *pm = g_advisor_bishop_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) != own)
                    ml.push(history.move(src, dst));
                dst = *pm++;
            }
        }
        //bishop
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint8 *pm = g_advisor_bishop_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) != own && xq.coordinate_is_empty((dst + src) >> 1))
                    ml.push(history.move(src, dst));
                dst = *pm++;
            }
        }
        //rook
        for (uint i = 0; i < 2; ++i)
        {
            uint dst;
            src = xq.piece(++idx);
            if (src == InvaildCoordinate)
                continue;
            dst = xq.nonempty_left_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = dst, dst = coordinate_left(src);
                    dst != tmp;
                    dst = coordinate_left(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_right_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = dst, dst = coordinate_right(src);
                    dst != tmp;
                    dst = coordinate_right(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_down_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = dst, dst = coordinate_down(src);
                    dst != tmp;
                    dst = coordinate_down(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_up_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = dst, dst = coordinate_up(src);
                    dst != tmp;
                    dst = coordinate_up(dst))
                ml.push(history.move(src, dst));
        }
        //knight
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint16 *pm = g_kinght_moves[src];
            dst = *pm++;
            //23130 = ((InvaildCoordinate << 8) | InvaildCoordinate)
            while (dst != 23130UL)
            {
                uint leg = (dst & 0xff00) >> 8;
                dst &= 0xff;
                if (xq.coordinate_is_empty(leg) && xq.coordinate_color(dst) != own)
                    ml.push(history.move(src, dst));
                dst = *pm++;
            }
        }
        //cannon
        for (uint i = 0; i < 2; ++i)
        {
            uint dst;
            src = xq.piece(++idx);
            if (src == InvaildCoordinate)
                continue;
            dst = xq.nonempty_left_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = xq.nonempty_left_1(src), dst = coordinate_left(src);
                    dst != tmp;
                    dst = coordinate_left(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_right_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = xq.nonempty_right_1(src), dst = coordinate_right(src);
                    dst != tmp;
                    dst = coordinate_right(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_down_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = xq.nonempty_down_1(src), dst = coordinate_down(src);
                    dst != tmp;
                    dst = coordinate_down(dst))
                ml.push(history.move(src, dst));
            dst = xq.nonempty_up_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst));
            for (uint tmp = xq.nonempty_up_1(src), dst = coordinate_up(src);
                    dst != tmp;
                    dst = coordinate_up(dst))
                ml.push(history.move(src, dst));
        }
    }

    void generate_capture_moves(const XQ& xq, MoveList &ml, const History& history)
    {
        uint own = xq.player();
        uint opp = 1 - own;
        uint idx;
        uint src, dst;
        ml.clear();
        if (own == Red)
        {
            idx = RedKingIndex;
            //red king
            src = xq.piece(RedKingIndex);
            const uint8 *pm = g_red_king_pawn_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) == opp)
                    ml.push(history.move(src, dst, RedKingIndex, xq.coordinate(dst)));
                dst = *pm++;
            }
            //red pawn
            for (uint i = RedPawnIndex1; i <= RedPawnIndex5; ++i)
            {
                src = xq.piece(i);
                if (src == InvaildCoordinate)
                    continue;
                pm = g_red_king_pawn_moves[src];
                dst = *pm++;
                while (dst != InvaildCoordinate)
                {
                    if (xq.coordinate_color(dst) == opp)
                        ml.push(history.move(src, dst, i, xq.coordinate(dst)));
                    dst = *pm++;
                }
            }
        }
        else
        {
            idx = BlackKingIndex;
            //black king
            src = xq.piece(BlackKingIndex);
            const uint8 *pm = g_black_king_pawn_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) == opp)
                    ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
                dst = *pm++;
            }
            //black pawn
            for (uint i = BlackPawnIndex1; i <= BlackPawnIndex5; ++i)
            {
                src = xq.piece(i);
                if (src == InvaildCoordinate)
                    continue;
                pm = g_black_king_pawn_moves[src];
                dst = *pm++;
                while (dst != InvaildCoordinate)
                {
                    if (xq.coordinate_color(dst) == opp)
                        ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
                    dst = *pm++;
                }
            }
        }
        //advisor
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint8 *pm = g_advisor_bishop_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) == opp)
                    ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
                dst = *pm++;
            }
        }
        //bishop
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint8 *pm = g_advisor_bishop_moves[src];
            dst = *pm++;
            while (dst != InvaildCoordinate)
            {
                if (xq.coordinate_color(dst) == opp && xq.coordinate_is_empty((dst + src) >> 1))
                    ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
                dst = *pm++;
            }
        }
        //rook
        for (uint i = 0; i < 2; ++i)
        {
            uint dst;
            src = xq.piece(++idx);
            if (src == InvaildCoordinate)
                continue;
            dst = xq.nonempty_left_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_right_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_down_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_up_1(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
        }
        //knight
        for (uint i = 0; i < 2; ++i)
        {
            ++idx;
            src = xq.piece(idx);
            if (src == InvaildCoordinate)
                continue;
            const uint16 *pm = g_kinght_moves[src];
            dst = *pm++;
            //23130 = ((InvaildCoordinate << 8) | InvaildCoordinate)
            while (dst != 23130UL)
            {
                uint leg = (dst & 0xff00) >> 8;
                dst &= 0xff;
                if (xq.coordinate_is_empty(leg) && xq.coordinate_color(dst) == opp)
                    ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
                dst = *pm++;
            }
        }
        //cannon
        for (uint i = 0; i < 2; ++i)
        {
            uint dst;
            src = xq.piece(++idx);
            if (src == InvaildCoordinate)
                continue;
            dst = xq.nonempty_left_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_right_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_down_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
            dst = xq.nonempty_up_2(src);
            if (xq.coordinate_color(dst) == opp)
                ml.push(history.move(src, dst, idx, xq.coordinate(dst)));
        }
    }

    uint32 Generator::next()
    {
        switch (m_stage)
        {
        case 0:
            m_stage = 1;
            if (m_hash_move)
                return m_hash_move;
        case 1:
            m_stage = 2;
            m_index = 0;
            generate_moves(m_xq, m_ml, m_history);
        case 2:
            while (m_index < m_ml.size())
            {
                uint32 move = m_ml[m_index];
                if (is_good_cap(m_xq, move))
                {
                    m_ml[m_index] = 0;
                    m_index++;
                    return move;
                }
                m_index++;
            }
            m_stage = 3;
            m_index = 0;
        case 3:
            while (m_index < 2)
            {
                uint move = m_killer.killer(m_index);
                m_index++;
                if (move &&
                        m_xq.coordinate_color(move_src(move)) == m_xq.player() &&
                        is_legal_move(m_xq, move_src(move), move_dst(move)))
                    return move;
            }
            m_stage = 4;
            m_index = 0;
        case 4:
            while (m_index < m_ml.size())
            {
                uint32 move = m_ml[m_index];
                if (!move)
                {
                    m_index++;
                    continue;
                }
                for (uint j = m_index + 1; j < m_ml.size(); ++j)
                {
                    if (m_ml[j] > move)
                    {
                        m_ml[m_index] = m_ml[j];
                        m_ml[j] = move;
                        move = m_ml[m_index];
                    }
                }
                m_index++;
                return move;
            }
        }
        return 0;
    }

}//namespace folium
