/*
 * Copyright (C) 2013 Naver Corp. All rights reserved.
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

#ifndef _PTHREADNP_PTHREAD_H_
#define _PTHREADNP_PTHREAD_H_

#if defined(WIN32)
#include "pthreads-win32/pthread.h"
#define PTHREAD_CALLBACK PTW32_CDECL

#undef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX  (pthread_key_t)_POSIX_THREAD_KEYS_MAX

#if !defined(PTW32_STATIC_LIB)
#if defined(BUILDING_PTHREADNP)
#define PTHREADNP_EXPORT __declspec (dllexport)
#else
#define PTHREADNP_EXPORT __declspec (dllimport)
#endif
#else
#define PTHREADNP_EXPORT
#endif

#elif defined(ANDROID)
#include <../../usr/include/pthread.h>
#define PTHREAD_CALLBACK
#define PTHREADNP_EXPORT
#endif

/* returns a initialized pthread_once_t_ */
PTHREADNP_EXPORT pthread_once_t pthread_once_t_init();

#undef PTHREAD_ONCE_INIT
#define PTHREAD_ONCE_INIT   (pthread_once_t_init())

/* initializes current thread as main thread */
PTHREADNP_EXPORT int       pthread_init_main_np(void);

/* identifies if current thread is the main thread */
PTHREADNP_EXPORT int       pthread_main_np(void);

typedef struct pthread_main_np_t_ * pthread_main_np_t;

/* gets identifier of the main thread */
PTHREADNP_EXPORT pthread_main_np_t pthread_get_main_np(void);

/* initializes current thread as worker thread */
PTHREADNP_EXPORT void      pthread_init_current_np(pthread_main_np_t);

/* overrides standard pthread_create, maintains thread hierarchy */
PTHREADNP_EXPORT int       pthread_create_np(pthread_t *thread, pthread_attr_t const * attr,
                                    void *(PTHREAD_CALLBACK *start_routine)(void *), void * arg);

#if defined(OVERRIDE_PTHREAD_CREATE) && !defined(BUILDING_PTHREADNP)
#define pthread_create pthread_create_np
#endif

typedef struct pthread_shared_key_t_ * pthread_shared_key_t;

/* handles thread shared, which is shared between a main thread and its child threads */
PTHREADNP_EXPORT int       pthread_shared_key_create_np(pthread_shared_key_t* key);
PTHREADNP_EXPORT int       pthread_shared_key_delete_np(pthread_shared_key_t key);
PTHREADNP_EXPORT int       pthread_setshared_np(pthread_shared_key_t key, const void * value);
PTHREADNP_EXPORT void *    pthread_getshared_np(pthread_shared_key_t key);

#if defined(WIN32)
/* sets thread's name */
PTHREADNP_EXPORT void      pthread_setname_np(const char* name);

static bool operator==(const pthread_t& a, const pthread_t& b) { return a.p == b.p; }
static bool operator!=(const pthread_t& a, const pthread_t& b) { return a.p != b.p; }
static bool operator!(const pthread_t& a) { return !a.p; }
#endif

/* sets a user-defined function as callback */
PTHREADNP_EXPORT void      pthread_setusercallback_np(void (PTHREAD_CALLBACK *callback)(void *));

/* invokes the user-defined callback function */
PTHREADNP_EXPORT void      pthread_callusercallback_np(void *);

#endif /* _PTHREADNP_PTHREAD_H_ */
