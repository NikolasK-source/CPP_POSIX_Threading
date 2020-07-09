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
#include "Semaphore.hpp"

#include <cerrno>
#include <stdexcept>
#include <sysexits.h>

#include "common_header/defines.hpp"
#include "common_header/sysexcept.hpp"
#include "common_header/destructor_exception.hpp"
#include "pthread_timeout.hpp"

namespace de {
namespace Koesling {
namespace Threading {

Semaphore::Semaphore(unsigned int value) :
        max_value(value),
        thread_queue(0)
{
    if(!value) throw std::logic_error(__CURRENT_FUNCTION__ +
            ": initializing a semaphore with maximum value of 0 is pointless.");

    sysexcept(sem_init(&sem, 0, value), "sem_init", errno);
}

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

Semaphore::Semaphore(Semaphore &&other) noexcept :
        sem(std::move(other.sem)),
        locking_threads(std::move(other.locking_threads)),
        max_value(std::move(other.max_value)),
        current_value(std::move(other.current_value)),
        thread_queue(std::move(other.thread_queue))
{ }

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
        throw std::logic_error(__CURRENT_FUNCTION__ + ": Double wait within one thread is not allowed.");

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
        throw std::logic_error(__CURRENT_FUNCTION__ + ": Double wait within one thread is not allowed.");

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
        throw std::logic_error(__CURRENT_FUNCTION__ + ": Double wait within one thread is not allowed.");

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
        throw std::logic_error(__CURRENT_FUNCTION__ +
                ": Releasing a semaphore which the thread does not hold is not allowed.");

    sysexcept(sem_post(&sem), "sem_post", errno);

    current_value++;
    locking_threads[thread] = false;
}

unsigned long Semaphore::get_source_version( ) noexcept
{
    return SEMAPHORE_VERSION;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
