/*
 * \file Semaphore.cpp
 * \brief Source file de::Koesling::Threading::Semaphore
 *
 * required compiler options:
 *          -std=c++11 (or higher)
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2020 Nikolas Koesling
 *
 */

// -------------------- non standard library includes ------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include "Semaphore.hpp"

#include "pthread_timeout.hpp"
#include "sysexcept.hpp"
#include "destructor_exception.hpp"


// -------------------- standard library includes ----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#include <cerrno>
#include <stdexcept>
#include <sysexits.h>
#include <iostream>


namespace de {
namespace Koesling {
namespace Threading {

// -------------------- Initialize static attributes -------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

std::ostream* Semaphore::error_stream = &std::cerr;


// -------------------- Constructor(s) ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Semaphore::Semaphore(unsigned int value) :
        max_value(value),
        thread_queue(0)
{
    if(!value) throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
            ": initializing a semaphore with maximum value of 0 is pointless.");

    sysexcept(sem_init(&sem, 0, value), "sem_init", errno);
}

Semaphore::Semaphore(Semaphore &&other) noexcept :
        sem(std::move(other.sem)),
        locking_threads(std::move(other.locking_threads)),
        max_value(std::move(other.max_value)),
        current_value(std::move(other.current_value)),
        thread_queue(std::move(other.thread_queue))
{ }


// -------------------- Destructor -------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Semaphore::~Semaphore( )
{
    try
    {
        sysexcept(sem_destroy(&sem), "sem_destroy", errno);
    }
    catch (const std::system_error& e)
    {
        destructor_exception_terminate(e, *error_stream, EX_OSERR);
    }
}


// -------------------- Methods ----------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

Semaphore& Semaphore::operator=(Semaphore &&other) noexcept
{
    if (&other != this) // check for self assignment
    {
        this->sem = std::move(other.sem);
        this->locking_threads = std::move(other.locking_threads);
        this->max_value = std::move(other.max_value);
        this->current_value = std::move(other.current_value);
        this->thread_queue = std::move(other.thread_queue);
    }
    
    return *this;
}

void Semaphore::wait( )
{
    auto thread = pthread_self();

    if(locking_threads[thread])
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": Double wait within one thread is not allowed.");

    thread_queue++;

    sysexcept(sem_wait(&sem), "sem_wait", errno);

    current_value++;
    thread_queue--;
    locking_threads[thread] = true;
}

bool Semaphore::trywait( )
{
    auto thread = pthread_self();

    if(locking_threads[thread])
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": Double wait within one thread is not allowed.");

    thread_queue++;

    auto temp = sem_wait(&sem);
    if(temp)
    {
        if(errno == EAGAIN)
        {
            thread_queue--;
            return false;
        }
        sysexcept(true, "sem_wait", errno);
    }

    current_value++;
    thread_queue--;
    locking_threads[thread] = true;

    return true;
}

bool Semaphore::timedwait(const timespec &time)
{
    auto thread = pthread_self();

    if(locking_threads[thread])
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": Double wait within one thread is not allowed.");

    auto timeout = pthread_timeout(time);

    thread_queue++;

    auto temp = sem_timedwait(&sem, &timeout);
    if(temp)
    {
        if(errno == ETIMEDOUT)
        {
            thread_queue--;
            return false;
        }
        sysexcept(true, "sem_wait", errno);
    }

    current_value++;
    thread_queue--;
    locking_threads[thread] = true;

    return true;
}

void Semaphore::post( )
{
    auto thread = pthread_self();

    if(!locking_threads[thread])
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) +
                ": Releasing a semaphore which the thread does not hold is not allowed.");

    sysexcept(sem_post(&sem), "sem_post", errno);

    current_value--;
    locking_threads[thread] = false;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
