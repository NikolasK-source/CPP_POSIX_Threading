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

#define RW_LOCK_VERSION 001000000UL    //!< Library version

namespace de {
namespace Koesling {
namespace Threading {

/*! \brief RW_Lock based on pthread_rw_lock
 *
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
        virtual ~RW_Lock( );

        //! Copying not allowed for objects of this type
        RW_Lock(RW_Lock &other) = delete;
        //! Copying not allowed for objects of this type
        RW_Lock& operator=(RW_Lock &other) = delete;

        //! Move object
        RW_Lock(RW_Lock &&other) noexcept;
        //! Move object
        RW_Lock& operator=(RW_Lock &&other) noexcept;

        void rd_lock( );
        void wr_lock( );
        bool rd_trylock( );
        bool wr_trylock( );
        bool rd_timedlock(const struct timespec &time);
        bool wr_timedlock(const struct timespec &time);

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

        /*! \brief get the version of the header file
         *
         * only interesting if used as library.
         */
        inline static unsigned long getHeaderVersion( ) noexcept;

        /*! \brief get the version of the source file
         *
         * only interesting if used as library.
         */
        static unsigned long getSourceVersion( ) noexcept;
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

inline unsigned long RW_Lock::getHeaderVersion( ) noexcept
{
    return RW_LOCK_VERSION;
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
