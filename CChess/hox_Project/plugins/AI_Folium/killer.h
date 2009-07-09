#ifndef _KILLER_H_
#define _KILLER_H_
namespace folium
{
    class Killer
    {
    public:
        void clear()
        {
            moves[0] = moves[1] = 0;
        }
        void push(uint move)
        {
            assert(move < 0x4000);
            if (move==moves[0])
                return;
            moves[1] = moves[0];
            moves[0] = static_cast<uint16>(move);
        }
        uint killer(uint i)
        {
            return moves[i];
        }
    private:
        uint16 moves[2];
    };

}//namespace folium

#endif //_KILLER_H_
