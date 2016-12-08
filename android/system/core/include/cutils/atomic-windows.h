
#ifndef ANDROID_CUTILS_ATOMIC_WINDOWS_H
#define ANDROID_CUTILS_ATOMIC_WINDOWS_H

#ifdef WIN32

#include "atomic.h"
#include <windows.h>

int32_t android_atomic_inc(volatile int32_t* addr)
{
    int32_t retval = *addr;
    InterlockedIncrement(addr);
    return retval;
}

int32_t android_atomic_dec(volatile int32_t* addr)
{
    int32_t retval = *addr;
    InterlockedDecrement(addr);
    return retval;
}

int32_t android_atomic_add(int32_t value, volatile int32_t* addr)
{
    int32_t retval = *addr;
    InterlockedExchangeAdd(addr, value);
    return retval;
}

int32_t android_atomic_and(int32_t value, volatile int32_t* addr)
{
    int32_t retval = *addr;
    _InterlockedAnd(addr, value);
    return retval;
}

int32_t android_atomic_or(int32_t value, volatile int32_t* addr)
{
    int32_t retval = *addr;
    _InterlockedOr(addr, value);
    return retval;
}

int32_t android_atomic_acquire_load(volatile const int32_t* addr)
{
    int32_t retval = *addr;
    MemoryBarrier();
    return retval;
}

int32_t android_atomic_release_load(volatile const int32_t* addr)
{
    int32_t retval = *addr;
    MemoryBarrier();
    return retval;
}

void android_atomic_acquire_store(int32_t value, volatile int32_t* addr)
{
    *addr = value;
    MemoryBarrier();
}

void android_atomic_release_store(int32_t value, volatile int32_t* addr)
{
    *addr = value;
    MemoryBarrier();
}

int32_t android_atomic_swap(int32_t value, volatile int32_t* addr)
{
    int32_t retval = *addr;
    InterlockedExchange(addr, value);
    return retval;
}

int android_atomic_acquire_cas(int32_t oldvalue, int32_t newvalue,
        volatile int32_t* addr)
{
    return !(InterlockedCompareExchangeAcquire(addr, newvalue, oldvalue) == oldvalue);
}

int android_atomic_release_cas(int32_t oldvalue, int32_t newvalue,
        volatile int32_t* addr)
{
    return !(InterlockedCompareExchangeRelease(addr, newvalue, oldvalue) == oldvalue);
}

#endif

#endif /* ANDROID_CUTILS_ATOMIC_WINDOWS_H */
