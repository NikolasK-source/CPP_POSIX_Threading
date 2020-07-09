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

#define SEMAPHORE_VERSION 001000000UL    //!< Library version

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
class Semaphore
{
    private:
        sem_t sem;
        std::unordered_map<pthread_t, bool> locking_threads;
        unsigned int max_value;
        volatile unsigned int current_value;
        volatile unsigned int thread_queue;

        //! error message stream for "non-throwable" errors
        static std::ostream* error_stream;

    public:
        Semaphore(unsigned int value);
        virtual ~Semaphore( );

        //! Copying not allowed for objects of this type
        Semaphore(Semaphore &other) = delete;
        //! Copying not allowed for objects of this type
        Semaphore& operator=(Semaphore &other) = delete;

        Semaphore(Semaphore &&other) noexcept;
        Semaphore& operator=(Semaphore &&other) noexcept;

        void wait();
        bool trywait();
        bool timedwait(const timespec &time);

        void post();

        inline unsigned int get_current_value( ) const noexcept;
        inline unsigned int get_thread_queue( ) const noexcept;
        inline unsigned int get_max_value( ) const noexcept;

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

inline unsigned long Semaphore::get_header_version( ) noexcept
{
    return SEMAPHORE_VERSION;
}

inline void Semaphore::set_error_stream(std::ostream& stream) noexcept
{
    error_stream = &stream;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */
