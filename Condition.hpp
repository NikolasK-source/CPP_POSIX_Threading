/*
 * \file Condition.hpp
 * \brief Header file de::Koesling::Threading::Condition
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

#pragma once

#include <pthread.h>
#include <string>
#include <ostream>

#define CONDITION_VERSION 001000000UL               //!< Library version

namespace de {
namespace Koesling {
namespace Threading {

    /*! \brief Condition variable based on pthread_cond
     *
     * A condition (short for ``condition variable'') is a synchronization
     * device that allows threads to suspend execution and relinquish the
     * processors until some predicate on shared data is satisfied. The basic
     * operations on conditions are:
     * signal the condition (when the predicate becomes true), and wait for the
     * condition, suspending the thread execution until another thread signals
     * the condition. (see man pthread_cond_xxx)
     */
    class Condition
    {
        private:
            /*! \brief Associated mutex
             *
             * A condition variable must always be associated with a mutex, to
             * avoid the race condition where a thread prepares to wait on a
             * condition variable and another thread signals the condition just
             * before the first  thread  actually waits on it.
             * (see man pthread_cond_xxx)
             */
            pthread_mutex_t mutex;

            //! condition variable
            pthread_cond_t condition;

            //! signal indicator to avoid spurious wakeup
            volatile bool signal_created;

            //! Number of threads waiting for this condition
            volatile size_t waiting_thread_count;

            //! indicates whether signal or broadcast was used
            volatile bool wakeup_by_brodcast;

            //! error message stream for "non-throwable" errors
            static std::ostream* error_stream;

        public:
            //! Create a new Condition object
            Condition( );

            //! Destroy a Condition object
            virtual ~Condition( );

            //! Copying not allowed for objects of this type
            Condition(Condition &other) = delete;
            //! Copying not allowed for objects of this type
            Condition& operator=(Condition &other) = delete;

            //! Move object
            Condition(Condition &&other) noexcept;
            //! Move object
            Condition& operator=(Condition &&other) noexcept;

            /*! \brief Waits for the condition variable to be signaled.
             *
             * The thread execution is suspended and does not consume any CPU
             * time until the condition variable is signaled.
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *                          - pthread_mutex_unlock
             *                          - pthread_cond_wait
             */
            bool wait( );

            /*! Waits for defined time span for the condition variable to be
             *  signaled.
             *
             * The thread execution is suspended and does not consume any CPU
             * time until the condition variable is signaled.
             * The thread is continued even if the condition variable is not
             * signaled, once the given time period has expired.
             *
             * attributes
             *   - time : time span
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *                          - pthread_mutex_unlock
             *                          - pthread_cond_timedwait
             *                          - gettimeofday
             *   - std::invalid_arg. : the given time span is invalid.
             */
            bool wait(const struct timespec &time);

            /*! Restarts one of the threads that are waiting on the condition
             *  variable.
             *
             * If no threads are waiting on the condition variable, nothing
             * happens. If several threads are waiting on the condition
             * variable, exactly one is restarted, but it is not specified
             * which.
             *
             * return value: true:  at least one thread could be signaled
             *               false: no thread is waiting for this condition
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *                          - pthread_mutex_unlock
             *                          - pthread_cond_signal
             */
            bool signal( );

            /*! \brief Restarts all the threads that are waiting on the
             *         condition variable.
             *
             * Nothing happens if no threads are waiting on the condition
             * variable.
             *
             * return value: true:  at least one thread could be signaled
             *               false: no thread is waiting for this condition
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *                          - pthread_mutex_unlock
             *                          - pthread_cond_broadcast
             */
            bool broadcast( );

            //! Set stream for error output for "non-throwable" errors
            inline static void set_error_stream(std::ostream& stream) noexcept;

            /*! \brief get the version of the header file
             *
             * only interesting if used as library.
             */
            inline static unsigned long get_header_version( ) noexcept;

            /*! \brief get the version of the source file
             *
             * only interesting if used as library.
             */
            static unsigned long get_source_version( ) noexcept;
    };

    inline unsigned long Condition::get_header_version( ) noexcept
    {
        return CONDITION_VERSION;
    }

    inline void Condition::set_error_stream(std::ostream& stream) noexcept
    {
        error_stream = &stream;
    }

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */

#ifndef __EXCEPTIONS
static_assert(false, "Exceptions are mandatory.");
#endif
