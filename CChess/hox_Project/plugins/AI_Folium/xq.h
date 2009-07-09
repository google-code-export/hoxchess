#ifndef _XQ_H_
#define _XQ_H_

#include <string>

#include "defines.h"
#include "move_helper.h"
#include "bitmap.h"
#include "xq_data.h"

namespace folium
{
    using std::string;

    class XQ
    {
    public:
        XQ();
        XQ(XQ const &);

        string get_fen()const;
        bool set_fen(const string&);

        uint coordinate(uint)const;
        uint coordinate_color(uint)const;
        uint coordinate_flag(uint)const;
        bool coordinate_is_empty(uint)const;

        uint piece(uint)const;

        uint player()const;

        bool do_move(uint, uint);
        void undo_move(uint, uint, uint);
        void do_null();
        void undo_null();

        uint nonempty_up_1(uint)const;
        uint nonempty_down_1(uint)const;
        uint nonempty_left_1(uint)const;
        uint nonempty_right_1(uint)const;
        uint nonempty_down_2(uint)const;
        uint nonempty_up_2(uint)const;
        uint nonempty_left_2(uint)const;
        uint nonempty_right_2(uint)const;
        uint distance(uint, uint)const;
        uint distance_is_0(uint, uint)const;
        uint distance_is_1(uint, uint)const;
    private:
        void clear();
    private:
        Bitmap m_bitmap;
        uint8 m_pieces[34];
        uint8 m_coordinates[91];
        uint8 m_player;
    };
    uint player_in_check(const XQ& xq, uint player);
    uint status(const XQ& xq);
    bool is_good_cap(const XQ& xq, uint move);
    bool is_legal_move(const XQ& xq, uint32 src, uint32 dst);
    string mirror4fen(const string& fen, uint mirror);

    inline uint XQ::coordinate(uint idx)const
    {
        assert (idx < 91UL);
        return m_coordinates[idx];
    }
    inline uint XQ::coordinate_color(uint sq)const
    {
        return piece_color(coordinate(sq));
    }
    inline uint XQ::coordinate_flag(uint sq)const
    {
        return piece_flag(coordinate(sq));
    }
    inline bool XQ::coordinate_is_empty(uint sq)const
    {
        return coordinate(sq) == EmptyIndex;
    }
    inline uint XQ::piece(uint idx)const
    {
        assert (idx < 34UL);
        return m_pieces[idx];
    }

    inline uint32 XQ::player()const
    {
        return m_player;
    }

    inline void XQ::do_null()
    {
        m_player = 1 - m_player;
    }
    inline void XQ::undo_null()
    {
        m_player = 1 - m_player;
    }

    inline uint XQ::nonempty_up_1(uint sq)const
    {
        return m_bitmap.nonempty_up_1(sq);
    }
    inline uint XQ::nonempty_up_2(uint sq)const
    {
        return m_bitmap.nonempty_up_2(sq);
    }
    inline uint XQ::nonempty_down_1(uint sq)const
    {
        return m_bitmap.nonempty_down_1(sq);
    }
    inline uint XQ::nonempty_down_2(uint sq)const
    {
        return m_bitmap.nonempty_down_2(sq);
    }
    inline uint XQ::nonempty_right_1(uint sq)const
    {
        return m_bitmap.nonempty_right_1(sq);
    }
    inline uint XQ::nonempty_right_2(uint sq)const
    {
        return m_bitmap.nonempty_right_2(sq);
    }
    inline uint XQ::nonempty_left_1(uint sq)const
    {
        return m_bitmap.nonempty_left_1(sq);
    }
    inline uint XQ::nonempty_left_2(uint sq)const
    {
        return m_bitmap.nonempty_left_2(sq);
    }
    inline uint XQ::distance(uint src, uint dst)const
    {
        return m_bitmap.distance(src, dst);
    }
    inline uint XQ::distance_is_0(uint src, uint dst)const
    {
        return m_bitmap.distance_is_0(src, dst);
    }
    inline uint XQ::distance_is_1(uint src, uint dst)const
    {
        return m_bitmap.distance_is_1(src, dst);
    }

    inline bool XQ::do_move(uint src, uint dst)
    {
        uint32 src_piece = coordinate(src);
        uint32 dst_piece = coordinate(dst);
        assert (src == piece(src_piece));
        assert (dst_piece == EmptyIndex || dst == piece(dst_piece));
        assert (piece_color(src_piece) == m_player);

        m_bitmap.do_move(src, dst);
        m_coordinates[src] = static_cast<uint8>(EmptyIndex);
        m_coordinates[dst] = static_cast<uint8>(src_piece);
        m_pieces[src_piece] = static_cast<uint8>(dst);
        m_pieces[dst_piece] = static_cast<uint8>(InvaildCoordinate);
        if (player_in_check(*this, m_player))
        {
            m_bitmap.undo_move(src, dst, dst_piece);
            m_coordinates[src] = static_cast<uint8>(src_piece);
            m_coordinates[dst] = static_cast<uint8>(dst_piece);
            m_pieces[src_piece] = static_cast<uint8>(src);
            m_pieces[dst_piece] = static_cast<uint8>(dst);
            return false;
        }

        m_player = 1UL - m_player;
        return true;
    }
    inline void XQ::undo_move(uint src, uint dst, uint dst_piece)
    {
        uint32 src_piece = coordinate(dst);
        assert (coordinate_is_empty(src));
        assert (piece(src_piece) == dst);

        m_bitmap.undo_move(src, dst, dst_piece);
        m_coordinates[src] = static_cast<uint8>(src_piece);
        m_coordinates[dst] = static_cast<uint8>(dst_piece);
        m_pieces[src_piece] = static_cast<uint8>(src);
        m_pieces[dst_piece] = static_cast<uint8>(dst);
        m_player = 1UL - m_player;

        assert (piece_color(src_piece) == m_player);
		assert (is_legal_move(*this, src, dst));
    }

}//namespace folium

#endif    //_XQ_H_
