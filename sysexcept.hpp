/*
 * \file sysexcept.hpp
 * \brief exception handling of system errors
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

#pragma once

#include <system_error>
#include <sysexits.h>
#include <typeinfo>

/*! Throws a std::system_error exception if condition is true
 *
 * arguments:
 *   - condition    : -true : exception (std::system_error) is thrown
 *                    -false: nothing
 *   - function name: string which contains the name of the system call that failed.
 *   - error_numer  : variable which contains the error number (e.g. the return value of the failed function or errno,
 *                                                                     see man page of system call for more information)
 */
#define sysexcept(condition, function_name, error_number) do                                                           \
{                                                                                                                      \
    if (condition)                                                                                                     \
        throw std::system_error(std::error_code((error_number), std::system_category( )),                              \
                std::string(__PRETTY_FUNCTION__) + " - " + (function_name));                                           \
}                                                                                                                      \
while (false)
