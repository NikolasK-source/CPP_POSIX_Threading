/*
 * \file RW_Lock.cpp
 * \brief Source file de::Koesling::Threading::RW_Lock
 *
 * required compiler options:
 *          -std=c++11 (or higher)
 *          -pthread
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2020 Nikolas Koesling
 *
 */

// -------------------- non standard library includes ------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include "RW_Lock.hpp"

#include "sysexcept.hpp"
#include "destructor_exception.hpp"
#include "pthread_timeout.hpp"

// -------------------- standard library includes ----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include <stdexcept>
#include <cerrno>
#include <sys/time.h>
#include <iostream>
#include <sysexits.h>


namespace de {
namespace Koesling {
namespace Threading {

// -------------------- Initialize static attributes -------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::ostream *RW_Lock::error_stream = &std::cerr;

// -------------------- Constructor(s) ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

RW_Lock::RW_Lock( ) noexcept :
        rw_lock(PTHREAD_RWLOCK_INITIALIZER), read_locked(0), write_locked(false)
{ }

RW_Lock::RW_Lock(RW_Lock &&other) noexcept :
        rw_lock(std::move(other.rw_lock)),
        read_locked(std::move(other.read_locked)),
        write_locked(std::move(other.write_locked))
{ }


// -------------------- Destructor -------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

RW_Lock::~RW_Lock( )
{
    if (is_locked()) // --> try to unlock
    {
        try
        {
            unlock( );
        }
        catch (const std::exception &e)
        {
            // is "ignored" here, but will most likely cause system call
            // 'pthread_mutex_destroy' to fail --> terminating anyway
            destructor_exception_continue(e, *error_stream);
        }
    }

    try // to destroy the pthread_rwlock object
    {
        auto temp = pthread_rwlock_destroy(&rw_lock);
        sysexcept(temp != 0, "pthread_rwlock_destroy", temp);
    }
    catch (const std::system_error &e) // Most likely if unlocking failed.
    {
        destructor_exception_terminate(e, *error_stream, EX_OSERR);
    }
}


// -------------------- Methods ----------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

RW_Lock& RW_Lock::operator=(RW_Lock &&other) noexcept
{
    if(&other != this)
    {
        this->rw_lock       = std::move(other.rw_lock);
        this->read_locked   = std::move(other.read_locked);
        this->write_locked  = std::move(other.write_locked);
    }

    return *this;
}

void RW_Lock::rd_lock( )
{
    auto temp = pthread_rwlock_rdlock(&rw_lock);
    sysexcept(temp != 0, "pthread_rwlock_rdlock", temp);

    read_locked++;
}

void RW_Lock::wr_lock( )
{
    auto temp = pthread_rwlock_wrlock(&rw_lock);
    sysexcept(temp != 0, "pthread_rwlock_wrlock", temp);

    write_locked = true;
}

bool RW_Lock::rd_trylock( )
{
    auto temp = pthread_rwlock_tryrdlock(&rw_lock);
    if (temp != 0)
    {
        if (temp == EBUSY) return false;
        sysexcept(true, "pthread_rwlock_tryrdlock", temp);
    }

    read_locked++;
    return true;
}

bool RW_Lock::wr_trylock( )
{
    auto temp = pthread_rwlock_trywrlock(&rw_lock);
    if (temp != 0)
    {
        if (temp == EBUSY) return false;
        sysexcept(true, "pthread_rwlock_trywrlock", temp);
    }

    write_locked = true;
    return true;
}

bool RW_Lock::rd_timedlock(const struct timespec &time)
{
    const struct timespec timeout_time = pthread_timeout(time);

    // lock rw_lock
    auto temp = pthread_rwlock_timedrdlock(&rw_lock, &timeout_time);
    if (temp != 0)
    {
        if (temp == ETIMEDOUT) return false;
        sysexcept(true, "pthread_rwlock_timedrdlock", temp);
    }

    read_locked++;
    return true;
}

bool RW_Lock::wr_timedlock(const struct timespec &time)
{
    const struct timespec timeout_time = pthread_timeout(time);

    // lock rw_lock
    auto temp = pthread_rwlock_timedwrlock(&rw_lock, &timeout_time);
    if (temp != 0)
    {
        if (temp == ETIMEDOUT) return false;
        sysexcept(true, "pthread_rwlock_timedwrlock", temp);
    }

    write_locked = true;
    return true;
}

void RW_Lock::unlock( )
{
    if (not is_locked( )) 
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": Call of RW_Lock::unlock(), but RW_Lock was never locked.");

    bool write_locked_temp = write_locked;

    if (write_locked) write_locked = false;
    else read_locked--;

    auto temp = pthread_rwlock_unlock(&rw_lock);
    if (temp != 0)
    {
        if (write_locked_temp) write_locked = true;
        else read_locked++;
        sysexcept(true, "pthread_rwlock_unlock", temp);
    }
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
