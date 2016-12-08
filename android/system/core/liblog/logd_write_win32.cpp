
#include "android/log.h"

#if defined(WIN32)

#include <stdio.h>
#include <cutils/threads.h>

static const char* prio_string[] = { "U", "N", "V", "D", "I", "W", "E", "F", "S" };

static char log_buffer[64 * 1024];
static mutex_t log_mutex = MUTEX_INITIALIZER;

class AutoMutexLocker
{
public:
    AutoMutexLocker(mutex_t* m)
        : m_lock(m) {
        mutex_lock(m_lock);
    }
    ~AutoMutexLocker() {
        mutex_unlock(m_lock);
    }

private:
    mutex_t* m_lock;
};

static void sprintf_alog_header(char* buffer, int prio, const char *tag)
{
    SYSTEMTIME system_time;
    ::GetLocalTime(&system_time);
    sprintf(buffer, "%02d-%02d %02d:%02d:%02d.%03d: %s/%s(%d): ", system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute, system_time.wSecond, system_time.wMilliseconds, prio_string[prio], (tag) ? tag : "" , ::GetCurrentThreadId());
}

int __android_log_write(int prio, const char *tag, const char *text)
{
    AutoMutexLocker lock(&log_mutex);
    sprintf_alog_header(log_buffer, prio, tag);
    strcat(log_buffer, text);
    strcat(log_buffer, "\r\n");
    ::OutputDebugStringA(log_buffer);
    return 0;
}

int __android_log_print(int prio, const char *tag,  const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(prio, tag, fmt, args);
    va_end(args);
    return 0;
}

int __android_log_vprint(int prio, const char *tag,
                         const char *fmt, va_list ap)
{
    AutoMutexLocker lock(&log_mutex);
    sprintf_alog_header(log_buffer, prio, tag);
    char* endptr = log_buffer + strlen(log_buffer);
    vsprintf(endptr, fmt, ap);
    strcat(log_buffer, "\r\n");
    ::OutputDebugStringA(log_buffer);
    return 0;
}

void __android_log_assert(const char *cond, const char *tag, const char *fmt, ...)
{
    ::OutputDebugStringA("ASSERT: ");
    ::OutputDebugStringA(cond);
    ::OutputDebugStringA("\r\n");
    va_list args;
    va_start(args, fmt);
    __android_log_print(ANDROID_LOG_FATAL, tag, fmt, args);
    va_end(args);
}

#endif
