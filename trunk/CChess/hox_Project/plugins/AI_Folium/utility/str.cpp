#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include "str.h"

namespace folium
{
    vector<string> split(const string& str, const char* seps)
    {
        vector<string> vec;
        boost::split(vec, str, boost::is_any_of(seps));
        return vec;
    }
    string join(const vector<string>& strs, const char* seps)
    {
        string str;
        for (vector<string>::const_iterator itr = strs.begin(); itr != strs.end();)
        {
            str += *itr;
            ++itr;
            if (itr != strs.end())
                str += seps;
        }
        return str;
    }
    string swapcase(const string& str)
    {
        string ret;
        for (string::const_iterator itr = str.begin(); itr != str.end(); ++itr)
        {
            if (isupper(*itr))
                ret.push_back(tolower(*itr));
            else if (islower(*itr))
                ret.push_back(toupper(*itr));
            else
                ret.push_back(*itr);
        }
        return ret;
    }
    string reverse(const string& str)
    {
        return string(str.rbegin(), str.rend());
    }
    string trim(const string& str)
    {
        string ret = str;
        boost::trim(ret);
        return ret;
    }
    bool endswith(const string& str, const string& end)
    {
        return boost::ends_with(str, end);
    }
}
