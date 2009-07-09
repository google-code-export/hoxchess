#ifndef _FOLIUM_STR_H_
#define _FOLIUM_STR_H_
#include <vector>
#include <string>
namespace folium
{
    using std::vector;
    using std::string;
    vector<string> split(const string& str, const char* seps);
    string join(const vector<string>& strs, const char* seps);
    string swapcase(const string& str);
    string reverse(const string& str);
    string trim(const string& str);
    bool endswith(const string& str, const string& end);
}//namespace folium

#endif