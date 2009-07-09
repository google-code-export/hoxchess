#include <map>
#include "move_helper.h"
#include "xq_data.h"

namespace folium
{
    using std::map;

    uint32 ucci2move(const string& ucci)
    {
        int sx, sy, dx, dy;
        sx = ucci[0] - 'a';
        sy = ucci[1] - '0';
        dx = ucci[2] - 'a';
        dy = ucci[3] - '0';
        if (sx < 0 || sx > 8 || sy < 0 || sy > 9 ||
                dx < 0 || dx > 8 || dy < 0 || dy > 9)
        {
            return 0;
        }
        return create_move(xy_coordinate(sx, sy), xy_coordinate(dx, dy));
    }

    string move2ucci(uint32 move)
    {
        string ucci;
        uint src = move_src(move);
        uint dst = move_dst(move);
        ucci.push_back(static_cast<char>(coordinate_x(src)+'a'));
        ucci.push_back(static_cast<char>(coordinate_y(src)+'0'));
        ucci.push_back(static_cast<char>(coordinate_x(dst)+'a'));
        ucci.push_back(static_cast<char>(coordinate_y(dst)+'0'));
        return ucci;
    }

    static const char _origin_1[] = "abcdefghi0123456789";
    static const char _target_1[] = "ihgfedcba0123456789";
    static const char _origin_2[] = "abcdefghi0123456789";
    static const char _target_2[] = "ihgfedcba9876543210";
    static map<char, char> _m1_map;
    static map<char, char> _m2_map;
    string mirror4uccimove(const string& ucci, uint mirror)
    {
        if (_m1_map.empty())
        {
            for (int i = 0; i < (sizeof(_origin_1) / sizeof(_origin_1[0])); ++i)
            _m1_map[_origin_1[i]] = _target_1[i];
            for (int i = 0; i < (sizeof(_origin_2) / sizeof(_origin_2[0])); ++i)
                _m2_map[_origin_2[i]] = _target_2[i];
        }
        assert(mirror < 4);
        string ret = ucci;
        if (mirror & 1)
        {
            for (string::iterator itr = ret.begin(); itr != ret.end(); ++itr)
                *itr = _m1_map[*itr];
        }
        if (mirror & 2)
        {
            for (string::iterator itr = ret.begin(); itr != ret.end(); ++itr)
                *itr = _m2_map[*itr];
        }
        return ret;
    }

}//namespace folium
