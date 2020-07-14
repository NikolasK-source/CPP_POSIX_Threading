/*
 * \file Mutex.hpp
 * \brief Header file de::Koesling::Threading::Mutex
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
#include <ostream>

namespace de {
namespace Koesling {
namespace Threading {

    /*! \brief Mutex based on pthread_mutex
     *
     * A mutex is a MUTual EXclusion device, and is useful for protecting shared
     * data structures from concurrent modifications, and implementing critical
     * sections and monitors.
     * A  mutex  has  two possible states: unlocked (not owned by any thread),
     * and locked (owned by one thread). A mutex can never be owned by two
     * different threads simultaneously. A thread attempting to lock a mutex
     * that is already locked by another thread is suspended until the owning
     * thread unlocks the mutex first. (cf. man pthread_mutex)
     */
    class Mutex final
    {
        private:
            //! pthread_mutex instance
            pthread_mutex_t mutex;

            //! ID of thread which has locked the mutex
            volatile pthread_t lock_thread;

            //! Availability indicator
            volatile bool locked;

            //! error message stream for "non-throwable" errors
            static std::ostream* error_stream;

        public:
            //! Create a new Mutex object
            Mutex( ) noexcept;

            //! Destroy a Mutex object
            virtual ~Mutex( );

            //! Copying not allowed for objects of this type
            Mutex(Mutex &other) = delete;
            //! Copying not allowed for objects of this type
            Mutex& operator=(Mutex &other) = delete;

            //! Move object
            Mutex(Mutex &&other) noexcept;
            //! Move object
            Mutex& operator=(Mutex &&other) noexcept;

            /*! \brief lock the mutex
             *
             * Locks the mutex. If the mutex is already locked, the calling
             * thread is blocked until the mutex is available.
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *   - std::logic_error: The mutex was locked twice in one thread
             */
            void lock( );

            /*! \brief unlock the mutex
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_unlock
             *   - std::logic_error: - The mutex is not locked
             *                       - The mutex is locked by an other thread
             */
            void unlock( );

            /*! \brief mutex locking attempt
             *
             * return value: true : successful (mutex is locked)
             *               false: failed (mutex is locked by another thread)
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_mutex_lock
             *   - std::logic_error: The mutex was locked twice in one thread
             */
            bool trylock( );

            /*! \brief mutex locking attempt (timed)
             *
             * Locks the mutex. If the mutex is already locked, the calling
             * thread is blocked until the mutex is available or the defined
             * time expired.
             *
             * return value: true : successful (mutex is locked)
             *               false: failed (timeout expired)
             *
             * possible throws:
             *   - std::system_error:     A system call failed. An error number
             *                            is set according to <cerrno>.
             *                            possible error numbers see man pages:
             *                               - pthread_mutex_lock
             *   - std::logic_error:      The mutex was locked twice in one
             *                            thread
             *   - std::invalid_argument: time value invalid
             */
            bool timedlock(const struct timespec &time);

            //! Check if the mutex is currently locked
            inline bool is_locked( ) const noexcept;

            //! Set stream for error output for "non-throwable" errors
            inline static void set_error_stream(std::ostream& stream) noexcept;
    };

    inline bool Mutex::is_locked( ) const noexcept
    {
        return locked;
    }

    inline void Mutex::set_error_stream(std::ostream& stream) noexcept
    {
        error_stream = &stream;
    }

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */

#ifndef __EXCEPTIONS
static_assert(false, "Exceptions are mandatory.");
#endif
