/*
 * \file pthread_timeout.cpp
 * \brief Calculation of the absolute time point for the timeout of pthread_*_timed* functions.
 *
 * required compiler options:
 *          -std=c++11 (or higher)
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2019 Nikolas Koesling
 *
 */

#include "pthread_timeout.hpp"
#include "common_header/defines.hpp"
#include "common_header/sysexcept.hpp"
#include <stdexcept>

namespace de {
namespace Koesling {
namespace Threading {

timespec pthread_timeout(const timespec& ts)
{
    // verify time
    if (ts.tv_sec < 0 || ts.tv_nsec < 0 || ts.tv_nsec >= NSEC_PER_SEC)
        throw std::invalid_argument(__CURRENT_FUNCTION__ + ": invalid timespec");

    // create time for timeout
    struct timeval current_time;
    sysexcept(gettimeofday(&current_time, nullptr), "gettimeofday", errno);

    struct timespec timeout_time;

    timeout_time.tv_sec = ts.tv_sec + current_time.tv_sec;
    timeout_time.tv_nsec = ts.tv_nsec + current_time.tv_usec * NSEC_PER_USEC;

    if (timeout_time.tv_nsec >= NSEC_PER_SEC)
    {
        timeout_time.tv_sec++;
        timeout_time.tv_nsec -= NSEC_PER_SEC;
    }

    return timeout_time;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
