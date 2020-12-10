/*
 * \file pthread_timeout.hpp
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

#include <sys/time.h>

namespace de {
namespace Koesling {
namespace Threading {

timespec pthread_timeout(const timespec& ts);

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
