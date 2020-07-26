/*
 * \file RW_Lock.hpp
 * \brief Header file de::Koesling::Threading::RW_Lock
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

#pragma once

#include <pthread.h>
#include <ostream>

namespace de {
namespace Koesling {
namespace Threading {

/*! \brief RW_Lock based on pthread_rw_lock
 *
 * Similar to Mutex:
 *      unlimited number of simultaneous readers
 *      maximum one simultaneous writer
 */
class RW_Lock final
{
    private:
        //! pthread_rw_lock instance
        pthread_rwlock_t rw_lock;

        //! Availability indicator
        volatile size_t read_locked;

        //! Availability indicator
        volatile bool write_locked;

        //! error message stream for "non-throwable" errors
        static std::ostream *error_stream;

    public:
        //! Create a new RW_Lock object
        RW_Lock( ) noexcept;

        //! Destroy a RW_Lock object
        ~RW_Lock( );

        //! Copying not allowed for objects of this type
        RW_Lock(RW_Lock &other) = delete;
        //! Copying not allowed for objects of this type
        RW_Lock& operator=(RW_Lock &other) = delete;

        //! Move object
        RW_Lock(RW_Lock &&other) noexcept;
        //! Move object
        RW_Lock& operator=(RW_Lock &&other) noexcept;

        //! read lock
        void rd_lock( );

        //! write lock
        void wr_lock( );

        //! try read lock
        bool rd_trylock( );

        //! try write lock
        bool wr_trylock( );

        //! timed read lock
        bool rd_timedlock(const struct timespec &time);

        //! timed write lock
        bool wr_timedlock(const struct timespec &time);

        //! unlock the rw lock
        void unlock( );

        //! Check if the RW_Lock is currently locked
        inline bool is_locked( ) noexcept;

        /*! \brief Check if the RW_Lock is currently locked to read
         *
         * return value: number of readers
         */
        inline size_t is_read_locked( ) noexcept;

        //! Check if the RW_Lock is currently locked to write
        inline bool is_write_locked( ) noexcept;

        //! Set stream for error output for "non-throwable" errors
        inline static void set_error_stream(std::ostream &stream) noexcept;
};

inline bool RW_Lock::is_locked( ) noexcept
{
    return read_locked || write_locked;
}

inline size_t RW_Lock::is_read_locked( ) noexcept
{
    return read_locked;
}

inline bool RW_Lock::is_write_locked( ) noexcept
{
    return write_locked;
}

inline void RW_Lock::set_error_stream(std::ostream &stream) noexcept
{
    error_stream = &stream;
}

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */

#ifndef __EXCEPTIONS
static_assert(false, "Exceptions are mandatory.");
#endif
