/*
 * \file destructor_exception.hpp
 * \brief Error messages for exceptions in noexcept destructors
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
#include <ostream>

/*! \brief Generates a error message for exceptions in constructors and terminates the program.
 *
 * attributes:
 *   - exception: the exception which was caught in the destructor.
 *   - ostream  : an output stream for the error message
 *   - exit_code: code with which the program terminates (e.g. EX_OSERR, EX_SOFTWARE, EX_CONFIG ...)
 */
#define destructor_exception_terminate(exception, ostream, exit_code) do                                               \
{                                                                                                                      \
    ostream << std::string(__PRETTY_FUNCTION__) << ": ";                                                               \
    ostream << "Exception of type " << typeid(exception).name( ) << " occurred in destructor of class "                \
            << typeid( *this).name( ) << ":" << std::endl << "    " << exception.what( ) << std::endl;                 \
    ostream << "Program terminating." << std::endl << std::flush;                                                      \
    exit(exit_code);                                                                                                   \
}                                                                                                                      \
while (false)

/*! \brief Generates a error message for exceptions in constructors (and continue)
 *
 * attributes:
 *   - exception: the exception (std::exception or child class) which was caught in the destructor.
 *   - ostream  : an output stream for the error message
 */
#define destructor_exception_continue(exception, ostream) do                                                           \
{                                                                                                                      \
    ostream << std::string(__PRETTY_FUNCTION__) << ": ";                                                               \
    ostream << "Exception of type " << typeid(exception).name( ) << " occurred in destructor of class "                \
            << typeid( *this).name( ) << ":" << std::endl << "    " << exception.what( ) << std::endl;                 \
    ostream << "An attempt is made to continue executing the destructor." << std::endl;                                \
}                                                                                                                      \
while (false)
