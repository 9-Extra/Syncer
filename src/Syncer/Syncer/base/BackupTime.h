#pragma once

#include <chrono>
#include <time.h>
#include <format>
namespace Syncer {
    using SyTimePoint = std::chrono::system_clock::time_point;
    
    inline std::string to_loacltime(const SyTimePoint point){
        time_t t = SyTimePoint::clock::to_time_t(point);
        struct tm tm;
        localtime_s(&tm, &t);
        
        return std::format("{}年{}月{}日 {}:{}:{}", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
}