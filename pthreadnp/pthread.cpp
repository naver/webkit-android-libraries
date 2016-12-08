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

#include "pthread.h"

#include <assert.h>
#include <memory>
#include <set>

#if defined(WIN32)
#define HTHREAD(pth) ((void*)(pth).p)
#define THREAD_LOCAL __declspec(thread)
#else
#define HTHREAD(pth) ((void*)(pth))
#define THREAD_LOCAL thread_local
#endif

#if defined(WIN32)
#include <windows.h>

// http://blogs.msdn.com/b/stevejs/archive/2005/12/19/505815.aspx
//
// Usage: SetThreadName(-1, "MainThread");
//
struct THREADNAMEINFO
{
    DWORD dwType; // must be 0x1000
    LPCSTR szName; // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags; // reserved for future use, must be zero
};

static void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
    THREADNAMEINFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
    }
}
#endif

typedef void* thread_t;
typedef size_t thread_shared_t;

static const size_t thread_shared_key_max = 1024;

struct pthread_main_thread_data {
    pthread_mutex_t main_thread_mutex;
    intptr_t* thread_shared_storage;
    thread_shared_t thread_shared_key_next;
    std::set<thread_shared_t> thread_shared_key_deleted;
    size_t child_threads;
};

struct pthread_thread_data {
    pthread_main_thread_data* main_thread_data;
    bool is_main_thread;
};

static THREAD_LOCAL pthread_thread_data* thread_data;

static pthread_thread_data& safe_thread_data()
{
    if (!thread_data)
        pthread_init_main_np();
    return *thread_data;
}

struct pthread_main_thread_lock {
    pthread_main_thread_lock() { pthread_mutex_lock(&thread_data->main_thread_data->main_thread_mutex); }
    ~pthread_main_thread_lock() { pthread_mutex_unlock(&thread_data->main_thread_data->main_thread_mutex); }
};

static pthread_key_t thread_data_key;
static pthread_once_t thread_once = PTHREAD_ONCE_INIT;

static void pthread_thread_destructor(void * data)
{
    // Some pthreads implementations zero out the thread specifics before calling destructor, so we temporarily reset it.
    thread_data = (pthread_thread_data*)data;

    {
        pthread_main_thread_lock lock;
        --thread_data->main_thread_data->child_threads;
    }

    if (thread_data->is_main_thread) {
        free(thread_data->main_thread_data->thread_shared_storage);
        pthread_mutex_destroy(&thread_data->main_thread_data->main_thread_mutex);
        delete thread_data->main_thread_data;
        thread_data->main_thread_data = nullptr;
    }

    delete thread_data;
    thread_data = nullptr;
}

static void pthread_init_current_once()
{
    pthread_key_create(&thread_data_key, pthread_thread_destructor);
}

static void pthread_init_current_private(pthread_main_thread_data* main_thread_data)
{
    thread_data = new pthread_thread_data { main_thread_data, false };
    pthread_setspecific(thread_data_key, thread_data);

    pthread_main_thread_lock lock;
    ++main_thread_data->child_threads;
}

/* returns a initialized pthread_once_t_ */
pthread_once_t pthread_once_t_init()
{
#if defined(WIN32)
    const static pthread_once_t i = { PTW32_FALSE, 0, 0, 0 };
    return i;
#else
    return 0;
#endif
}

/* initializes current thread as main thread */
int         pthread_init_main_np(void)
{
    if (thread_data) {
        if (thread_data->is_main_thread)
            return 0;
        return -1;
    }

    pthread_once(&thread_once, pthread_init_current_once);

    const size_t thread_shared_storage_size = sizeof(intptr_t) * (thread_shared_key_max + 1);
    pthread_main_thread_data* main_thread_data = new pthread_main_thread_data { {}, (intptr_t*)malloc(thread_shared_storage_size), 1, {}, 0 };
    pthread_mutex_init(&main_thread_data->main_thread_mutex, NULL);
    memset(main_thread_data->thread_shared_storage, 0, thread_shared_storage_size);

    thread_data = new pthread_thread_data { main_thread_data, true };
    pthread_setspecific(thread_data_key, thread_data);
    return 0;
}

/* identifies if current thread is the main thread */
int         pthread_main_np(void)
{
    return thread_data->is_main_thread;
}

/* gets identifier of the main thread */
pthread_main_np_t pthread_get_main_np(void)
{
    return reinterpret_cast<pthread_main_np_t>(thread_data->main_thread_data);
}

/* initializes current thread as worker thread */
void        pthread_init_current_np(pthread_main_np_t main)
{
    if (thread_data)
        return;

    pthread_init_current_private(reinterpret_cast<pthread_main_thread_data*>(main));
}

struct pthread_create_np_data {
    thread_t main_thread;
    void *(PTHREAD_CALLBACK *start_routine)(void *);
    void * start_routine_arg;
};

static void * PTHREAD_CALLBACK pthread_create_np_entry(void * arg)
{
    std::unique_ptr<pthread_create_np_data> data(reinterpret_cast<pthread_create_np_data *>(arg));

    pthread_init_current_private(reinterpret_cast<pthread_main_thread_data*>(data->main_thread));

    return data->start_routine(data->start_routine_arg);
}

/* overrides standard pthread_create, maintains thread hierarchy */
int         pthread_create_np(pthread_t *thread, pthread_attr_t const * attr,
                    void *(PTHREAD_CALLBACK *start_routine)(void *), void * arg)
{
    pthread_create_np_data * data = new pthread_create_np_data { safe_thread_data().main_thread_data, start_routine, arg };
    return pthread_create(thread, attr, pthread_create_np_entry, data);
}

int         pthread_shared_key_create_np(pthread_shared_key_t* key)
{
    if (!key)
        return -1;

    pthread_main_thread_lock lock;
    thread_shared_t shared_key;
    if (thread_data->main_thread_data->thread_shared_key_deleted.empty()) {
        assert(thread_data->main_thread_data->thread_shared_key_next < thread_shared_key_max);
        shared_key = thread_data->main_thread_data->thread_shared_key_next++;
    } else {
        auto deleted_value = thread_data->main_thread_data->thread_shared_key_deleted.begin();
        shared_key = *deleted_value;
        thread_data->main_thread_data->thread_shared_key_deleted.erase(deleted_value);
    }

    *key = reinterpret_cast<pthread_shared_key_t>(shared_key);
    return 0;
}

int         pthread_shared_key_delete_np(pthread_shared_key_t key)
{
    if (!key)
        return -1;

    thread_shared_t shared_key = reinterpret_cast<thread_shared_t>(key);
    pthread_main_thread_lock lock;
    thread_data->main_thread_data->thread_shared_storage[shared_key] = 0;
    thread_data->main_thread_data->thread_shared_key_deleted.insert(shared_key);
    return 0;
}

int         pthread_setshared_np(pthread_shared_key_t key, const void * value)
{
    if (!key)
        return -1;

    thread_shared_t shared_key = reinterpret_cast<thread_shared_t>(key);
    thread_data->main_thread_data->thread_shared_storage[shared_key] = (intptr_t)value;
    return 0;
}

void *      pthread_getshared_np(pthread_shared_key_t key)
{
    if (!key)
        return 0;

    thread_shared_t shared_key = reinterpret_cast<thread_shared_t>(key);
    return (void *)thread_data->main_thread_data->thread_shared_storage[shared_key];
}

#if defined(WIN32)
/* sets thread's name */
void        pthread_setname_np(const char* name)
{
    if (!::IsDebuggerPresent())
        return;

    SetThreadName(::GetCurrentThreadId(), name);
}
#endif

static void empty_usercallback(void *) { }

static void (PTHREAD_CALLBACK *s_usercallback)(void *) = empty_usercallback;

void        pthread_setusercallback_np(void (PTHREAD_CALLBACK *callback)(void *))
{
    s_usercallback = (!callback) ? empty_usercallback : callback;
}

void        pthread_callusercallback_np(void * arg)
{
    s_usercallback(arg);
}
