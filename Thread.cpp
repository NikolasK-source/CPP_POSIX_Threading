/*
 * \file Thread.cpp
 * \brief Source file de::Koesling::Threading::Thread
 *
  * required compiler options:
 *          -std=c++11 (or higher)
 *          -pthread
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2019 Nikolas Koesling
 *
 */

#include "Thread.hpp"
#include <system_error>
#include <csignal>
#include <cerrno>
#include <sys/time.h>
#include <iostream>
#include <sysexits.h>

#include "common_header/sysexcept.hpp"
#include "common_header/destructor_exception.hpp"

namespace de {
namespace Koesling {
namespace Threading {

std::ostream* Thread::error_stream = &std::cerr;

// ----------------------- error messages --------------------------------------
// =============================================================================
//! error message: invalid detachstate
#define INVALID_DETACHSTATE __CURRENT_FUNCTION__ + ": detachstate is invalid."

//! error message: double thread start
#define ALREADY_STARTED __CURRENT_FUNCTION__ + ": Call of Thread::start(), but thread was started "\
    "already."

//! error message: detach not joinable thread
#define DETACH_DETACHED __CURRENT_FUNCTION__ + ": Call of Thread::detach(), but thread is not joinable "\
    "or already detached."

//! error message: detach not running thread
#define DETACH_STOPPED __CURRENT_FUNCTION__ + ": Call of Thread::detach(), but thread is not running."

//! error message: join a detached thread
#define JOIN_DETACHED __CURRENT_FUNCTION__ + ": Call of Thread::join(), but thread is detached."

//! error message: join a thread that was not started
#define JOIN_NOT_STARTED __CURRENT_FUNCTION__ + ": Call of Thread::join(), nut thread is not started "\
    "or was killed"

//! error message: cancel a thread that was not started
#define CANCEL_STOPPED __CURRENT_FUNCTION__ + ": Call of Thread::kill(), but thread is not running."

//! error message: signal rise, but thread is not running
#define SIGNAL_STOPPED __CURRENT_FUNCTION__ + ": Call of Thread::signal(), but thread is not running."

Thread::Thread(thread_function_t function) :
        thread_id(0), funcion(function), running(false), detachstate(JOINABLE), arguments(nullptr)
{
    // create pthread_attr object ( + error handling )
    int temp = pthread_attr_init(&attributes);
    sysexcept(temp != 0, "pthread_attr_init", temp);
}

Thread::Thread(thread_function_t function, detachstate_t detachstate) :
        thread_id(0), funcion(function), running(false), detachstate(detachstate), arguments(nullptr)
{
    // create pthread_attr object ( + error handling )
    int temp = pthread_attr_init(&attributes);
    sysexcept(temp != 0, "pthread_attr_init", temp);

    // set detachestate of pthread ( + error handling )
    temp = pthread_attr_setdetachstate(&attributes, detachstate == JOINABLE ? PTHREAD_CREATE_JOINABLE :
            PTHREAD_CREATE_DETACHED);
    sysexcept(temp != 0, "pthread_attr_setdetachstate", temp);
}

Thread::Thread(thread_function_t function, const pthread_attr_t &attributes) :
        thread_id(0), funcion(function), running(false), attributes(attributes), arguments(nullptr)
{
    // get detachstate from attributes object ( + error handling )
    int temp_detachstate;
    int temp = pthread_attr_getdetachstate(&attributes, &temp_detachstate);
    sysexcept(temp != 0, "pthread_attr_getdetachstate", temp);

    // store detachstate
    detachstate = temp_detachstate == PTHREAD_CREATE_JOINABLE ? JOINABLE : DETACHED;
}

Thread::~Thread( )
{
    // kill thread if it is running and joinable, because joining is no longer
    //     possible.
    // running == true --> std::logic error can not be thrown.
    // handling of system_error is barely possible. However, this could only
    // occur in the event of major system errors.
    if (running && detachstate == JOINABLE)
    {
        try	// to cancel
        {
            cancel( );
        }
        catch (const std::system_error &e)
        {
            destructor_exception_terminate(e, *error_stream, EX_OSERR);
        }
    }

    try // to destroy the pthread_attr object
    {
        int temp = pthread_attr_destroy(&attributes);
        sysexcept(temp != 0, "pthread_attr_destroy", temp);
    }
    catch (const std::system_error &e)
    {
        /* Currently always successful on Linux!
         * (checked on linux version 2.6.32-431 and 5.3.0-40)
         * Error handling for reasons of portability
         */
        destructor_exception_terminate(e, *error_stream, EX_OSERR);
    }
}

Thread::Thread(Thread &&other) noexcept :
        thread_id(std::move(other.thread_id)),
        funcion(std::move(other.funcion)),
        running(std::move(other.running)),
        detachstate(std::move(other.detachstate)),
        attributes(std::move(other.attributes)),
        arguments(std::move(other.arguments))
{
}

Thread& Thread::operator =(Thread &&other) noexcept
{
    if (this != &other) // check for self assignment
    {
        this->thread_id = std::move(other.thread_id);
        this->funcion = std::move(other.funcion);
        this->running = std::move(other.running);
        this->detachstate = std::move(other.detachstate);
        this->attributes = std::move(other.attributes);
        this->arguments = std::move(other.arguments);
    }

    return *this;
}

void Thread::start( )
{
    // avoid double start; The function call would not fail, but the previous
    // thread id would be lost --> join impossible
    if (running) throw std::logic_error( ALREADY_STARTED);

    // create a new thread ( + error handling )
    int temp = pthread_create(&thread_id, &attributes, funcion, arguments);
    sysexcept(temp != 0, "pthread_create", temp);

    running = true;
}

void Thread::detach( )
{
    // thread is already detached --> useless call
    if (detachstate == DETACHED) throw std::logic_error( DETACH_DETACHED);

    // detach a stopped (joined, killed) thread
    if (!running) throw std::logic_error( DETACH_STOPPED);

    // detach the thread ( + error handling )
    // system error is very unlikely and, if it occurs, indicates major problems
    // (such as unintentional external overwriting of objects data).
    int temp = pthread_detach(thread_id);
    sysexcept(temp != 0, "pthread_detach", temp);

    // set new detachstate (detaching was successful)
    detachstate = DETACHED;
}

void Thread::join(void **return_value)
{
    // a detached thread cannot be joined
    if (detachstate == DETACHED) throw std::logic_error(JOIN_DETACHED);

    // a thread that was killed cannot be joined
    if (!running) throw std::logic_error(JOIN_NOT_STARTED);

    // join the thread ( + error handling )
    int temp = pthread_join(thread_id, return_value);
    sysexcept(temp != 0, "pthread_join", temp);

    running = false;
}

bool Thread::try_join(void **return_value)
{
    // a detached thread cannot be joined
    if (detachstate == DETACHED) throw std::logic_error(JOIN_DETACHED);

    // a thread that was killed cannot be joined
    if (!running) throw std::logic_error(JOIN_NOT_STARTED);

    // join the thread ( + error handling )
    int temp = pthread_tryjoin_np(thread_id, return_value);
    if (temp != 0)
    {
        if (temp == EBUSY) return false;
        sysexcept(true, "pthread_tryjoin_np", temp);
    }

    running = false;
    return true;
}

bool Thread::timed_join(const struct timespec &time, void **return_value)
{
    // a detached thread cannot be joined
    if (detachstate == DETACHED) throw std::logic_error(JOIN_DETACHED);

    // a thread that was killed cannot be joined
    if (!running) throw std::logic_error(JOIN_NOT_STARTED);

    // verify time
    if (time.tv_sec < 0 || time.tv_nsec < 0 || time.tv_nsec >= NSEC_PER_SEC) throw std::invalid_argument(
            "invalid timespec");

    // create time for timeout
    struct timeval current_time;
    sysexcept(gettimeofday(&current_time, nullptr), "gettimeofday", errno);

    struct timespec timeout_time;

    timeout_time.tv_sec = time.tv_sec + current_time.tv_sec;
    timeout_time.tv_nsec = time.tv_nsec + current_time.tv_usec *
    NSEC_PER_USEC;

    if (timeout_time.tv_nsec >= NSEC_PER_SEC)
    {
        timeout_time.tv_sec++;
        timeout_time.tv_nsec -= NSEC_PER_SEC;
    }

    // join the thread ( + error handling )
    int temp = pthread_timedjoin_np(thread_id, return_value, &timeout_time);
    if (temp != 0)
    {
        if (temp == ETIMEDOUT) return false;
        sysexcept(true, "pthread_tryjoin_np", temp);
    }

    running = false;
    return true;
}

void Thread::cancel( )
{
    // unable to cancel a stopped thread
    if (!running) throw std::logic_error( CANCEL_STOPPED);

    // cancel the thread ( + error handling )
    int temp = pthread_cancel(thread_id);
    sysexcept(temp != 0, "pthread_cancel", temp);

    running = false;
}

void Thread::send_signal(int signum)
{
    // unable to send a signal to a stopped thread
    if (!running) throw std::runtime_error( SIGNAL_STOPPED);

    // send signal to thread ( + error handling )
    int temp = pthread_kill(thread_id, signum);
    sysexcept(temp != 0, "pthread_kill", temp);
}

unsigned long Thread::get_source_version( ) noexcept
{
    return THREAD_VERSION;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
