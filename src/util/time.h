#ifndef _UTIL_TIME_H_
#define _UTIL_TIME_H_

#include <chrono>

inline uint64_t TimestampInMillisec() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline uint64_t TimestampInMicrosec() {
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

#endif // _UTIL_TIME_H_