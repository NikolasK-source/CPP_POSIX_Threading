# C++ POSIX Threading (pthread)

## General
The aim of this project is to make the use of pthread easier.
It provides classes for creating and synchronizing threads.

Errors that occur when calling the ptherad system functions are handled by throwing an
exception of type std::system_error.
In addition to checking the system calls for errors, there are some check if the function calls
are reasonable. For example, it is checked if a mutex is locked twice within a thread or if
an attempt is made to join a detached thread.

## Classes
### Thread
The class Thread allows the creation of a new thread. A thread object can be created by using three different constructors:

1. Thread(thread_function_t)
2. Thread(thread_function_t, detachstate_t)
3. Thread(thread_function_t, pthread_attr_t)

The method start() starts the thread with the attributes specified by the constructors.

If the thread is not already detached, it can be detached by calling the method detach().
If the thread is already detached or not started, a call of detach() causes an exception of type std::logic_error to be thrown.

If the thread is not detached, it must be joined.
Joining can be done with the three methods:

1. void join( void** )
2. bool try_join( void** )
3. bool timed_join( timespec&, void** )

All methods throw an exception of type std::logic_error if the thread is not joinable or was never started.
The void** argument of all three functions can be used to query the return value of the thread function.
It’s default value is a null pointer.

The method cancel() sends a cancellation request to the thread.
Whether and how the thread reacts to the request depends on its cancelability state and cancelability type.

### Mutex
The class implements a Mutex based on pthread_mutex.

The Mutex is locked by calling one of the following methods:

1. void lock()
2. bool trylock()
3. bool timedlock(timespec&)

The thread id of the locking thread is stored. This prevents double locking within one
thread and unlocking from another thread. If an attempt is made to double lock the Mutex,
an exception of type std::logic_error is thrown.

The Mehtod unlock() unlocks the Mutex.
If the Mutex is not locked or it is locked by another thread, a std::logic_error exception
is thrown.

### Condition

This class implements a condition variable based on pthread_cond.

The methods wait() and wait(timespec&) are used to wait for a condition to be signaled.
An optional passed parameter defines a maximum wait time.
If the condition was signaled, the methods returns true, otherwise they return false. 
If no timeout is specified, the method can not return without a condition being signaled and therefore always returns true.

The condition can be signaled by two methods: signal() and broadcast().
signal() is restarting exactly one of the threads waiting for the condition and broadcast() is restarting all.
Both methods return false if no thread was waiting for the condition, otherwise they return true.

### RW_Lock
Like Mutex, but implements a RW_Lock based on pthread_rwlock.

The RW_Lock is locked by calling one of the following methods:
1. void [rd|wr]_lock()
2. bool [rd|wr]_trylock()
3. bool [rd|wr]_timedlock(timespec&)

The Mehtod unlock() unlocks the RW_Lock.
