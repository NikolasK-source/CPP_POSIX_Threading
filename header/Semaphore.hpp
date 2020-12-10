/*
 * \file Semaphore.hpp
 * \brief Header file de::Koesling::Threading::Semaphore
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

#pragma once

#include <semaphore.h>
#include <pthread.h>
#include <unordered_map>

namespace de {
namespace Koesling {
namespace Threading {

/* \brief Semaphore based on POSIX semaphores
 *
 * POSIX semaphores allow processes and threads to synchronize their actions.
 *
 * A  semaphore  is  an integer whose value is never allowed to fall below zero.
 * Two operations can be performed on semaphores: increment the semaphore value by one (sem_post(3));
 * and decrement the semaphore value by one (sem_wait(3)).
 * If the  value  of  a  semaphore is currently zero, then a sem_wait(3) operation will block until the value becomes
 * greater than zero.
 */
class Semaphore final
{
    private:
        //! actual semaphore
        sem_t sem;

        //! map of all known threads (object scope), true if thread is locking this semaphore
        std::unordered_map<pthread_t, bool> locking_threads;

        //! maximum value for this semaphore
        unsigned int max_value;

        //! current value for this semaphore
        volatile unsigned int current_value;

        //! number of threads currently waiting for this semaphore
        volatile unsigned int thread_queue;

        //! error message stream for "non-throwable" errors
        static std::ostream* error_stream;

    public:
        /*! Create a new Semaphore
         *
         * attributes:
         *      - value: maximum value of this semaphore (== number of possible simultaneous accesses)
         *
         * possible throws:
         *      - std::invalid_argument: invalid value (only happens if 0 is passed)
         *      - std::system_error    : a system call failed
         */
        explicit Semaphore(unsigned int value);

        //! Destroy Object, not virtual because object is final and does not inherit
        ~Semaphore( );

        //! Copying not allowed for objects of this type
        Semaphore(Semaphore &other) = delete;
        //! Copying not allowed for objects of this type
        Semaphore& operator=(Semaphore &other) = delete;

        //! move everything to a new object
        Semaphore(Semaphore &&other) noexcept;

        //! move everything to a new object
        Semaphore& operator=(Semaphore &&other) noexcept;

        /*! wait for this semaphore (unlimited)
         *
         * possible throws:
         *      - std::logic_error : double wait within one thread
         *      - std::system_error: a system call failed
         */
        void wait();

        /*! try to get this semaphore
         *
         * return value:
         *      true : could get this semaphore
         *      false: this semaphore is currently not available
         *
         * possible throws:
         *      - std::logic_error : double wait within one thread
         *      - std::system_error: a system call failed
         */
        bool trywait();

        /*! wait for this semaphore (with timeout)
         *
         * attributes:
         *      - time: maximum time to wait for this semaphore
         *
         * return value:
         *      true : could get this semaphore
         *      false: this semaphore was not available within the specified time span
         *
         * possible throws:
         *      - std::logic_error     : double wait within one thread
         *      - std::invalid_argument: time span is invalid
         *      - std::system_error    : a system call failed
         *
         */
        bool timedwait(const timespec &time);

        /*! post this semaphore
         *
         * possible throws:
         *      - std::logic_error : thread does not lock this semaphore
         *      - std::system_error: a system call failed
         */
        void post();

        //! get current value of this semaphore
        inline unsigned int get_current_value( ) const noexcept;

        //! get the number of threads waiting for this semaphore
        inline unsigned int get_thread_queue( ) const noexcept;

        //! get the maximum value of this semaphore
        inline unsigned int get_max_value( ) const noexcept;

        //! Set stream for error output for "non-throwable" errors
        inline static void set_error_stream(std::ostream& stream) noexcept;
};

inline unsigned int Semaphore::get_current_value( ) const noexcept
{
    return current_value;
}

inline unsigned int Semaphore::get_thread_queue( ) const noexcept
{
    return thread_queue;
}

inline unsigned int Semaphore::get_max_value( ) const noexcept
{
    return max_value;
}

inline void Semaphore::set_error_stream(std::ostream& stream) noexcept
{
    error_stream = &stream;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
