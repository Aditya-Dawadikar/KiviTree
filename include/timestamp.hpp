#pragma once

#include <chrono>

class Timestamp {
public:
    /// Returns current UNIX timestamp in milliseconds
    static long long now_ms() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }

    /// Returns current UNIX timestamp in seconds
    static long long now_sec() {
        using namespace std::chrono;
        return duration_cast<seconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }
};
