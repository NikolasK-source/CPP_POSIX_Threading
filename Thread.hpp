/*
 * \file Thread.hpp
 * \brief Header file de::Koesling::Threading::Thread
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

#define THREAD_VERSION 001000000UL  //!< Library version

namespace de {
namespace Koesling {
namespace Threading {

    //! Thread function type
    typedef void* (*thread_function_t)(void*);
    
    /*! \brief Create threads based on pthread
     *
     * This class provides easy handling of threads. These are created with the pthread_* interface.
     */
    class Thread final
    {
        public:
            //! possible thread states
            enum detachstate_t
            {
                JOINABLE, 	//!< Thread is joinable
                DETACHED	//!< Thread is detached
            };

        private:
            /*! \brief ID of the created thread.
             *
             * invalid until method start() is called.
             */
            pthread_t thread_id;

            //! function which is called by the created thread
            thread_function_t funcion;

            /*! \brief Thread status indicator
             *
             * TRUE: The thread might be running
             *
             * If the thread has been detached, it cannot be determined whether
             * it is still running.
             */
            bool running;

            /*! \brief detach state of this Thread
             *
             * JOINABLE or DETACHED
             */
            detachstate_t detachstate;

            /*! \brief Attributes for call of pthread_create(...)
             *
             * see 'man pthread_create' and 'man pthread_attr_*' for more
             * details.
             */
            pthread_attr_t attributes;

            /*! \brief Arguments passed to thread function when thread is
             *         started.
             *
             * The data is referenced and not copied!
             * It must therefore be ensured that the data is available during
             * the entire thread runtime.
             *
             * arguments are set with method setArguments(...);
             */
            void *arguments;

            //! error message stream for "non-throwable" errors
            static std::ostream* error_stream;

        public:
            /*! \brief Create a Thread with default attributes
             *
             * arguments:
             *   - function: Function of type 'void* function(void* arg)' which
             *               is called by the thread when started.
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_attr_init
             *
             * Thread attributes:
             *     Detach state        = PTHREAD_CREATE_JOINABLE
             *     Scope               = PTHREAD_SCOPE_SYSTEM
             *     Inherit scheduler   = PTHREAD_INHERIT_SCHED
             *     Scheduling policy   = SCHED_OTHER
             *     Scheduling priority = 0
             *     Guard size          = 4096 bytes
             *     Stack address       = 0x40196000
             *     Stack size          = 0x201000 bytes
             */
            explicit Thread(thread_function_t function);

            /*! \brief Create a Thread with default attributes except detach
             *  	   state
             *
             * arguments:
             *   - function:     Function of type 'void* function(void* arg)'
             *                   which is called by the thread when started.
             *   - detachstate_: Specifies whether the thread is jonable.
             *                   Vaild values: Thread::JOINABLE
             *                                 Thread::DETACHED
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man pages:
             *                          - pthread_attr_init
             *                          - pthread_attr_setdetachstate
             *
             * Thread attributes:
             *     Detach state        = <detachstate>
             *     Scope               = PTHREAD_SCOPE_SYSTEM
             *     Inherit scheduler   = PTHREAD_INHERIT_SCHED
             *     Scheduling policy   = SCHED_OTHER
             *     Scheduling priority = 0
             *     Guard size          = 4096 bytes
             *     Stack address       = 0x40196000
             *     Stack size          = 0x201000 bytes
             */
            Thread(thread_function_t function, detachstate_t detachstate);

            /*! \brief Create a Thread with custom attributes
             *
             * arguments:
             *   - function:   Function of type 'void* function(void* arg)'
             *                 which is called by the thread when started.
             *   - attributes: struct of type 'pthread_attr_t'
             *                 see 'man pthread_attr_*' for setup.
             *
             * possible throws:
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_attr_getdetachstate
             */
            Thread(thread_function_t function, const pthread_attr_t &attributes);

            /*! \brief Destroy the Thread object.
             *
             * Thread is terminated if running.
             * Program is terminated if system call fails. (unlikely)
             */
            virtual ~Thread( );

            //! Copy Constructor is forbidden. Use std::move(...)!
            Thread(Thread &other) = delete;

            //! Copy operator is forbidden. Use std::move(...)!
            Thread& operator=(Thread &other) = delete;

            //! Move all components to a new Thread object.
            Thread(Thread &&other) noexcept;

            //! Move all components to a new Thread object.
            Thread& operator=(Thread &&other) noexcept;

            /*! \brief Start the Thread
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_create
             */
            void start( );

            /*! \brief Detach a joninable thread.
             *
             * Do not use on detached threads!
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_detach
             */
            void detach( );

            /*! \brief Join a joninable thread.
             *
             * Do not use on detached threads!
             *
             * arguments:
             *   - return_value: pointer to a data location where the return
             *                   value of the thread function shall be stored.
             *                   Use nullptr to ignore.
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_join
             */
            void join(void **return_value = nullptr);

            /*! \brief Try to join a joninable thread.
             *
             * Do not use on detached threads!
             *
             * arguments:
             *   - return_value: pointer to a data location where the return
             *                   value of the thread function shall be stored.
             *                   Use nullptr to ignore.
             *
             * return value:  -true : success
             * 				  -false: failed (thread is still running)
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_join
             */
            bool try_join(void **return_value = nullptr);

            /*! \brief try to join a joninable thread. Block for passed time span
             *
             *  Do not use on detached threads!
             *
             * arguments:
             *   - return_value: pointer to a data location where the return
             *                   value of the thread function shall be stored.
             *                   Use nullptr to ignore.
             *
             * return value:  -true : success
             * 				  -false: failed (thread is still running)
             * 						  timeout expired
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_join
             */
            bool timed_join(const struct timespec &time, void **return_value = nullptr);

            /*! \brief Stops the thread immediately.
             *
             * Data corruption is possible. --> Avoid if somehow possible!
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_cancel
             */
            void cancel( );

            /*! \brief Send a signal to the thread.
             *
             * Handle with care! May cause program termination if no signal
             * handler is established for the given signal number
             *
             * arguments:
             *   - signum : Signal number according to <csignal>
             *
             * possible throws:
             *   - std::logic_error : Programming mistake. Should never happen
             *                        in a well programmed software.
             *                        A detailed error message is given.
             *   - std::system_error: A system call failed. An error number is
             *                        set according to <cerrno>.
             *                        possible error numbers see man page(s):
             *                          - pthread_cancel
             */
            void send_signal(int signum);

            /*! \brief Send a signal to the thread.
             *
             * see send_signal(...)
             */
            inline void kill(int signum);

            /*! \brief Set arguments for call of thread function
             *
             * see 'man pthread_create' and 'man pthread_attr_*' for more
             * details.
             */
            inline void set_arguments(void *arguments) noexcept;

            /*! \brief Get the ID of the thread.
             *
             * Undefined until call of method start()
             */
            inline pthread_t get_id( ) noexcept;

            //! Get a pointer to the thread function
            inline thread_function_t get_function( ) noexcept;

            /*! \brief Compare two Thread objects by ID.
             *
             * Will probably never be needed.
             * Only possible to be true by checking with itself.
             */
            inline bool operator==(const Thread &other) const noexcept;

            /*! \brief Compare this Thread with another pthread id
             *
             * Will probably never be needed.
             */
            inline bool operator==(const pthread_t &id) const noexcept;

            //! Check weather this is the actual thread
            inline bool is_my_thread( ) const noexcept;

            //! Get the actual detachstate of this thread
            inline detachstate_t get_detachstate( ) const noexcept;

            //! get the id of the actual thread.
            inline static pthread_t get_my_id( ) noexcept;

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

    // ------------------- Inline functions for class Thread -------------------

    inline void Thread::kill(int signum)
    {
        send_signal(signum);
    }

    inline void Thread::set_arguments(void *arguments) noexcept
    {
        this->arguments = arguments;
    }

    inline pthread_t Thread::get_id( ) noexcept
    {
        return thread_id;
    }

    inline thread_function_t Thread::get_function( ) noexcept
    {
        return funcion;
    }

    inline bool Thread::operator ==(const Thread &other) const noexcept
    {
        return thread_id == other.thread_id;
    }

    inline bool Thread::operator ==(const pthread_t &id) const noexcept
    {
        return thread_id == id;
    }

    inline bool Thread::is_my_thread( ) const noexcept
    {
        return thread_id == get_my_id( );
    }

    inline Thread::detachstate_t Thread::get_detachstate( ) const noexcept
    {
        return detachstate;
    }

    inline pthread_t Thread::get_my_id( ) noexcept
    {
        return pthread_self( );
    }

    inline unsigned long Thread::get_header_version( ) noexcept
    {
        return THREAD_VERSION;
    }

    inline void Thread::set_error_stream(std::ostream& stream) noexcept
    {
            error_stream = &stream;
    }

} /* namespace Threading */
} /* namespace Koesling */
} /* namespace de */

#ifndef __EXCEPTIONS
static_assert(false, "Exceptions are mandatory.");
#endif
