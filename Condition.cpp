/*
 * \file Condition.hpp
 * \brief Source file de::Koesling::Threading::Condition
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

#include "Condition.hpp"

#include <stdexcept>
#include <cerrno>
#include <sys/time.h>
#include <iostream>
#include <sysexits.h>

#include "common_header/sysexcept.hpp"
#include "common_header/destructor_exception.hpp"

#include "pthread_timeout.hpp"

namespace de {
namespace Koesling {
namespace Threading {

std::ostream* Condition::error_stream = &std::cerr;

// ignore old style cast, because PTHREAD_COND_INITIALIZER uses one
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
Condition::Condition( ) noexcept :
        mutex( PTHREAD_MUTEX_INITIALIZER),
        condition(PTHREAD_COND_INITIALIZER),
        signal_created(false),
        waiting_thread_count(0),
        wakeup_by_brodcast(false)
{ }
// re-enable warnings
#pragma GCC diagnostic pop

Condition::~Condition( )
{
    try
    {
        int temp = pthread_cond_destroy(&condition);
        sysexcept(temp != 0, "pthread_cond_destroy", temp);

        temp = pthread_mutex_destroy(&mutex);
        sysexcept(temp != 0, "pthread_mutex_destroy", temp);
    }
    catch (const std::system_error &e)
    {
        // failed to destroy mutex/condition --> major error --> terminate
        destructor_exception_terminate(e, *error_stream, EX_OSERR);
    }
}

Condition::Condition(Condition &&other) noexcept :
        mutex(std::move(other.mutex)),
        condition(std::move(other.condition)),
        signal_created(std::move(other.signal_created)),
        waiting_thread_count(std::move(other.waiting_thread_count)),
        wakeup_by_brodcast(std::move(other.wakeup_by_brodcast))
{ }

Condition& Condition::operator =(Condition &&other)
noexcept
{
    if (this != &other) // check for self assignment
    {
        this->mutex = std::move(other.mutex);
        this->condition = std::move(other.condition);
        this->signal_created = std::move(other.signal_created);
        this->waiting_thread_count = std::move(other.waiting_thread_count);
        this->wakeup_by_brodcast = std::move(other.wakeup_by_brodcast);
    }

    return *this;
}

bool Condition::wait( )
{
    // lock mutex (to avoid condition_signal/broadcast race condition)
    int temp = pthread_mutex_lock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_lock", temp);

    waiting_thread_count++;	// add waiting tread

    while (!signal_created)
    {
        temp = pthread_cond_wait(&condition, &mutex);
        sysexcept(temp != 0, "pthread_cond_wait", temp);
    }

    waiting_thread_count--;	// remove waiting thread

    // only reset if no other thread can receive the signal
    if (!wakeup_by_brodcast || waiting_thread_count == 0) signal_created = false;

    // unlock mutex
    temp = pthread_mutex_unlock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_unlock", temp);

    return true;
}

bool Condition::wait(const struct timespec &time)
{
    const struct timespec timeout_time = pthread_timeout(time);

    // lock mutex (avoid condition_signal/broadcast race condition)
    int temp = pthread_mutex_lock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_lock", temp);

    waiting_thread_count++; // add waiting tread

    bool return_value = true;
    while (!signal_created)
    {
        temp = pthread_cond_timedwait(&condition, &mutex, &timeout_time);
        if (temp != 0)
        {
            if (temp == ETIMEDOUT)
            {
                return_value = false;
                break;
            }
            else sysexcept(true, "pthread_cond_timedwait", temp);
        }
    }

    waiting_thread_count--; // remove waiting thread

    // only reset if no other thread can receive the signal
    if (!wakeup_by_brodcast || waiting_thread_count == 0) signal_created = false;

    // unlock mutex
    temp = pthread_mutex_unlock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_unlock", temp);

    return return_value;
}

bool Condition::signal( )
{
    // lock mutex (avoid condition_wait race condition)
    int temp = pthread_mutex_lock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_lock", temp);

    // no thread waiting ? --> no signal created
    signal_created = waiting_thread_count != 0;
    wakeup_by_brodcast = false;

    /* buffer the return value, because 'signal_created' could be reset (to false) by the
     * signaled thread before this function is completed.
     */
    bool ret_val = signal_created;

    // signal condition (wake exactly one thread)
    temp = pthread_cond_signal(&condition);
    sysexcept(temp != 0, "pthread_cond_signal", temp);

    // unlock mutex
    temp = pthread_mutex_unlock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_unlock", temp);

    return ret_val;
}

bool Condition::broadcast( )
{
    // lock mutex (avoid condition_wait race condition)
    int temp = pthread_mutex_lock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_lock", temp);

    // no thread waiting ? --> no signal created
    signal_created = waiting_thread_count != 0;
    wakeup_by_brodcast = true;

    /* buffer the return value, since 'signal_created' could be changed by the
     * signaled thread before this function is completed.
     */
    bool ret_val = signal_created;

    // broadcast condition (wake all threads)
    temp = pthread_cond_broadcast(&condition);
    sysexcept(temp != 0, "pthread_cond_broadcast", temp);

    // unlock mutex
    temp = pthread_mutex_unlock(&mutex);
    sysexcept(temp != 0, "pthread_mutex_unlock", temp);

    return ret_val;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
