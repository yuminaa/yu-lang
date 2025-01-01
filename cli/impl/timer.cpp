#include "../timer.h"
#include <iomanip>
#include <iostream>
#include "../style.h"

Timer::Timer(const std::string_view stage, const bool is_verbose, size_t *count) : start_time(clock::now())
                                                                                , stage_name(stage)
                                                                                , verbose(is_verbose)
                                                                                , counter(count)
{
    if (verbose)
    {
        std::cout << style::blue << "⟡ Starting " << stage_name << "..." << style::reset << "\n";
    }
}

Timer::~Timer()
{
    if (verbose)
    {
        const auto end = clock::now();
        const duration diff = end - start_time;
        std::cout << style::green << "  ✓ " << stage_name << " completed in "
                << std::fixed << std::setprecision(3)
                << diff.count() * 1000 << "ms" << style::reset;

        if (counter)
        {
            std::cout << " (" << *counter << " items processed)";
        }
        std::cout << "\n";
    }
}

void Timer::log(const std::string_view msg) const
{
    if (verbose)
    {
        std::cout << "  → " << msg << "\n";
    }
}