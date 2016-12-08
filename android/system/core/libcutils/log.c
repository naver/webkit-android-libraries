
#include "cutils/log.h"

#if !defined(WIN32)

int __android_log_buf_write(int bufID, int prio, const char *tag, const char *text)
{
    return 0;
}

int __android_log_buf_print(int bufID, int prio, const char *tag, const char *fmt, ...)
{
    return 0;
}

#endif