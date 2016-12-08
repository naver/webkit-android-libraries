/*
 * Copyright (C) 2015 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SAFEINT_H_INCLUDED_
#define _SAFEINT_H_INCLUDED_

#include "SafeInt3.hpp"

#include <limits>

#if !defined(THREAD_LOCAL)
#if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER
#define THREAD_LOCAL __declspec(thread)
#endif

#if SAFEINT_COMPILER == CLANG_COMPILER
#if __has_feature(cxx_thread_local)
#define THREAD_LOCAL thread_local
#else
#define THREAD_LOCAL __thread
#endif
#endif

#if SAFEINT_COMPILER == GCC_COMPILER
#define THREAD_LOCAL __thread
#endif
#endif

namespace safeint {

class exception {
public:
    static THREAD_LOCAL bool overflowed;
    static THREAD_LOCAL bool divisionByZero;
};

class exceptionhandler {
public:
    static void SAFEINT_STDCALL SafeIntOnOverflow()
    {
        exception::overflowed = true;
    }

    static void SAFEINT_STDCALL SafeIntOnDivZero()
    {
        exception::divisionByZero = true;
    }
};

template<typename T, typename U> T integral_cast(U&& value)
{
    static_assert(NumericType<T>::isInt, "Type T is not an integral type.");
    SafeInt<T, exceptionhandler> i(value);
    if (exception::overflowed)
        i = (value < 0) ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
    exception::overflowed = false;
    return i;
}

}

#endif // _SAFEINT_H_INCLUDED_
