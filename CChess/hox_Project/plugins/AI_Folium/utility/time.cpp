
#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#else
#  include <sys/resource.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include "time.h"

namespace folium
{
    double now_time()
    {
#if defined(_WIN32) || defined(_WIN64)
        return double(GetTickCount()) / 1000.0;
#else
        struct timeval tv[1];
        struct timezone tz[1];
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
        gettimeofday(tv,tz);
        return return double(tv->tv_sec) + double(tv->tv_usec) * 1E-6;;
#endif
    }
}
