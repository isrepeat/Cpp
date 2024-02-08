#include <Windows.h>
#include <iostream>
#include <chrono>

int main() {
    std::chrono::time_point currently = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now()
        );
    
    std::chrono::duration millis_since_utc_epoch = currently.time_since_epoch();

    return 0;
}