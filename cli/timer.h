#pragma once

#include <chrono>

class Timer
{
private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using duration = std::chrono::duration<double>;

    time_point start_time;
    std::string_view stage_name;
    bool verbose;
    size_t *counter;

public:
    void log(std::string_view msg) const;

    Timer(const std::string_view stage, bool is_verbose, size_t *count = nullptr);

    ~Timer();
};