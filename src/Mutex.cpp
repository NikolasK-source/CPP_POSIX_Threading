/*
 * \file Mutex.cpp
 * \brief Source file de::Koesling::Threading::Mutex
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

// -------------------- non standard library includes ------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include "Mutex.hpp"

#include "pthread_timeout.hpp"
#include "sysexcept.hpp"
#include "destructor_exception.hpp"


// -------------------- standard library includes ----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include <stdexcept>
#include <cerrno>
#include <sys/time.h>
#include <iostream>
#include <sysexits.h>


// -------------------- Macros -----------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

//! error message: unlock a not locked mutex
#define MUTEX_NOT_LOCKED std::string(__PRETTY_FUNCTION__) + ": Call of Mutex::unlock(), but Mutex was never locked"

//! error message: unlock a mutex from an other thread
#define LOCKING_THREAD_MISSMATCH std::string(__PRETTY_FUNCTION__) + ": Calling pthread_mutex_unlock() with a mutex "   \
    "that the calling thread does not hold will result in undefined behavior."

//! error message: double lock in one thread
#define DOUBLE_LOCK std::string(__PRETTY_FUNCTION__) + ": A mutex must not be locked twice in the same thread."


namespace de {
namespace Koesling {
namespace Threading {

// -------------------- Initialize static attributes -------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

std::ostream* Mutex::error_stream = &std::cerr;

// -------------------- Constructor(s) ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Mutex::Mutex( ) noexcept :
        mutex( PTHREAD_MUTEX_INITIALIZER),
        lock_thread(0),
        locked(false)
{ }

Mutex::Mutex(Mutex &&other) noexcept :
        mutex(std::move(other.mutex)),
        lock_thread(std::move(other.lock_thread)),
        locked(std::move(other.locked))
{ }


// -------------------- Destructor -------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Mutex::~Mutex( )
{
    if (locked) // --> try to unlock
    {
        try
        {
            unlock( );
        }
        catch (const std::logic_error &e)
        {
            // is "ignored" here, but will most likely cause system call
            // 'pthread_mutex_destroy' to fail --> terminating anyway
            destructor_exception_continue(e, *error_stream);
        }
        // system error is very unlikely and, if it occurs, indicates major
        // problems in the operating system
        catch (const std::system_error &e)
        {
            destructor_exception_terminate(e, *error_stream, EX_OSERR);
        }
    }

    try // to destroy the pthread_mutex object
    {
        auto temp = pthread_mutex_destroy(&mutex);
        sysexcept(temp != 0, "pthread_mutex_destroy", temp);
    }
    catch (const std::system_error &e) // Most likely if unlocking failed.
    {
        destructor_exception_terminate(e, *error_stream, EX_OSERR);
    }
}


// -------------------- Methods ----------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Mutex& Mutex::operator =(Mutex &&other) noexcept
{
    if (&other != this) // check for self assignment
    {
        this->mutex = std::move(other.mutex);
        this->lock_thread = std::move(other.lock_thread);
        this->locked = std::move(other.locked);
    }

    return *this;
}

void Mutex::lock( )
{
    // Double lock in one thread --> programming error
    if (locked && lock_thread == pthread_self( ))
    {
        throw std::logic_error(DOUBLE_LOCK);
    }

    // lock the mutex ( + error handling )
    auto temp = pthread_mutex_lock(&mutex);
    // system error is very unlikely and, if it occurs, indicates major
    // problems in the operating system
    sysexcept(temp != 0, "pthread_mutex_lock", temp);

    // save locking thread to prevent unlocking from other thread
    lock_thread = pthread_self( ); // pthread_self can't fail
    locked = true;
}

void Mutex::unlock( )
{
    // unlocking a unlocked mutex --> undefined behavior
    if (!locked) throw std::logic_error( MUTEX_NOT_LOCKED);

    // unlocking from an other thread --> undefined behavior
    if (pthread_self( ) != lock_thread) throw std::logic_error(LOCKING_THREAD_MISSMATCH);

    // prematurely set to false, because after sucessfull unlocking the mutex
    // could be immediately locked by another thread (even before the error
    // handling is completed and so the variable could provide a wrong
    // information about the locking state if set after unlocking.
    locked = false;

    // unlock mutex ( + error handling )
    auto temp = pthread_mutex_unlock(&mutex);
    if (temp != 0)
    {
        // unlocking mutex failed --> set variable back to true
        locked = true;
        sysexcept(true, "pthread_mutex_unlock", temp);
    }
}

bool Mutex::trylock( )
{
    // Double lock in one thread --> programming error
    if (locked && (lock_thread == pthread_self( ))) throw std::logic_error(DOUBLE_LOCK);

    // try to lock the mutex ( + error handling )
    auto temp = pthread_mutex_trylock(&mutex);
    if (temp != 0)
    {
        if (temp == EBUSY) return false; // mutex is busy --> trylock "failed"
        sysexcept(true, "pthread_mutex_trylock", temp); // other errors
    }

    // lock was successful --> save locking thread to prevent unlocking from
    //                         other tread
    lock_thread = pthread_self( );
    locked = true;

    return true;
}

bool Mutex::timedlock(const struct timespec &time)
{
    // Double lock in one thread --> programming error
    if (locked && (lock_thread == pthread_self( ))) throw std::logic_error(DOUBLE_LOCK);

    const struct timespec timeout_time = pthread_timeout(time);

    // try to lock the mutex ( + error handling )
    auto temp = pthread_mutex_timedlock(&mutex, &timeout_time);
    if (temp != 0)
    {
        if (temp == ETIMEDOUT) return false;
        sysexcept(true, "pthread_mutex_timedlock", temp);
    }

    // lock was successful --> save locking thread to prevent unlocking from other tread
    lock_thread = pthread_self( );
    locked = true;

    return true;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
