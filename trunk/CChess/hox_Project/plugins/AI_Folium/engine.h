#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <set>
#include <string>

#include "defines.h"
#include "movelist.h"
#include "xq_data.h"
#include "xq.h"
#include "history.h"
#include "hash.h"

namespace folium
{
    using std::set;
    using std::string;
    class Engine
    {
    public:
        Engine();
        virtual ~Engine() {}
        void setxq(const XQ&);
        bool load(const string& fen);
        string fen(){return m_xq.get_fen();}

        virtual bool readable() {return false;};
        virtual string readline(){return string();};
        virtual void writeline(const string& str){};

        bool make_move(uint32 move);
        void unmake_move();

        uint32 search(set<uint>);

        bool m_debug;
        bool m_stop;
        bool m_ponder;
        int m_depth;
        double m_starttime;
        double m_mintime;
        double m_maxtime;
    private:
        void interrupt();
        void do_null();
        void undo_null();
        bool is_legal_move(uint move);
        bool is_stop();
        int value()const;
        int loop_value(int)const;
        int full(int, int, int);
        int leaf(int, int);
        int quies(int, int);
        XQ m_xq;
        int m_ply;//current ply
        int m_start_ply;//start search ply
        int m_null_ply;//null move ply
        uint32 m_keys[512];
        uint64 m_locks[512];
        sint32 m_values[512];
        uint32 m_traces[512];
        History m_history;
        HashTable m_hash;
		volatile uint m_interrupt;

    private:
		uint m_tree_nodes;
		uint m_leaf_nodes;
		uint m_quiet_nodes;
		uint m_hash_hit_nodes;
		uint m_hash_move_cuts;
		uint m_kill_cuts_1;
		uint m_kill_cuts_2;
		uint m_null_nodes;
		uint m_null_cuts;

    };

    inline int Engine::value()const
    {
        return (m_xq.player() == Red ? m_values[m_ply] : - m_values[m_ply]) + 4;//pawn value = 9
    }

    inline int Engine::loop_value(int ply)const
    {
        if ((m_ply - m_null_ply) >= 4 && m_keys[m_ply] == m_keys[m_ply - 4])
        {
            if (trace_flag(m_traces[m_ply]) && trace_flag(m_traces[m_ply - 2]))
            {
                if (trace_flag(m_traces[m_ply - 1]) && trace_flag(m_traces[m_ply - 3]))
                    return static_cast<int>(value() * 0.9f);
                return INVAILDVALUE - (ply - 1);
            }
            else if (trace_flag(m_traces[m_ply - 1]) && trace_flag(m_traces[m_ply - 3]))
                return (ply - 1) - INVAILDVALUE;
            return static_cast<int>(value() * 0.9f);
        }
        return INVAILDVALUE;
    }

    inline bool Engine::is_legal_move(uint move)
    {
        return move &&  folium::is_legal_move(m_xq, move_src(move), move_dst(move)) && m_xq.player() == m_xq.coordinate_color(move_src(move));
    }

    inline void Engine::do_null()
    {
        assert(!trace_flag(m_traces[m_ply]));
        uint op, np;
        op = m_ply;
        np = m_ply + 1;
        m_keys[np] = m_keys[op];
        m_locks[np] = m_locks[op];
        m_values[np] = m_values[op];
        m_traces[np] = m_null_ply << 20;
        m_null_ply = m_ply = np;
        m_xq.do_null();
    }

    inline void Engine::undo_null()
    {
        m_null_ply = m_traces[m_ply--] >> 20;
        m_xq.undo_null();
    }

    inline bool Engine::is_stop()
    {
        m_interrupt = (m_interrupt + 1) & 8191;
        if (m_interrupt == 8191)
            interrupt();
        return m_stop;
    }
}//namespace folium

#endif //_ENGINE_H_
