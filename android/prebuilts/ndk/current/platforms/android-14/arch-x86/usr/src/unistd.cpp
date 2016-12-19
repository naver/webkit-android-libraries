/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "unistd.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "sys/select.h"
#include "sys/socket.h"
#include "win/unixfd.h"

#include <errno.h>

pid_t gettid(void)
{
    return ::GetCurrentThreadId();
}

int access(const char * pathname, int mode)
{
    return _access(pathname, mode);
}

int pipe(int * pipefd)
{
    return socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
}

int close(int fildes)
{
    return (fildes > 0 && UnixFD::get(fildes))?UnixFD::get(fildes)->close():EBADF;
}

off_t lseek(int fildes, off_t offset, int whence)
{
    return UnixFD::get(fildes)->lseek(offset, whence);
}

int read(int fildes, void * buf, size_t nbyte)
{
    return UnixFD::get(fildes)->read(buf, nbyte, 0);
}

int write(int fildes, const void * buf, size_t count)
{
    return UnixFD::get(fildes)->write(buf, count, 0);
}

int dup(int fildes)
{
    return (fildes > 0) ? UnixFD::get(fildes)->dup() : -1;
}

int dup2(int oldfd, int newfd)
{
    return (oldfd > 0) ? UnixFD::get(oldfd)->dup2(newfd) : -1;
}

int ioctl(int fd, int request, ssize_t* va)
{
    return ioctlsocket(fd, request, (u_long FAR *)va);
}

int ftruncate(int fildes, off_t length)
{
    return UnixFD::get(fildes)->chsize(length);
}

unsigned sleep(unsigned seconds)
{
    Sleep(seconds * 1000);
    return 0;
}

int  gethostname(char * name, size_t len)
{
    return FORWARD_CALL(GETHOSTNAME)(name, len);
}

long getpagesize(void)
{
    static long g_pagesize = 0;
    if (!g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}

int isatty(int fildes)
{
    return UnixFD::get(fildes)->isatty();
}

int __cdecl     chmod(_In_z_ const char * _Filename, int _AccessMode)
{
    return _chmod(_Filename, _AccessMode);
}

int __cdecl     chsize(_In_ int _FileHandle, _In_ long _Size)
{
    return UnixFD::get(_FileHandle)->chsize(_Size);
}

int __cdecl     eof(_In_ int _FileHandle)
{
    return UnixFD::get(_FileHandle)->eof();
}

long __cdecl    filelength(_In_ int _FileHandle)
{
    return UnixFD::get(_FileHandle)->filelength();
}

int __cdecl     locking(_In_ int _FileHandle, _In_ int _LockMode, _In_ long _NumOfBytes)
{
    return UnixFD::get(_FileHandle)->locking(_LockMode, _NumOfBytes);
}

char * __cdecl  mktemp(_Inout_z_ char * _TemplateName)
{
    return _mktemp(_TemplateName);
}

int __cdecl     setmode(_In_ int _FileHandle, _In_ int _Mode)
{
    return UnixFD::get(_FileHandle)->setmode(_Mode);
}

int __cdecl     sopen(const char * _Filename, _In_ int _OpenFlag, _In_ int _ShareFlag, ...)
{
    va_list args;
    va_start(args, _ShareFlag);
    int _Mode = va_arg(args, int);
    int retval = UnixFD::sopen(_Filename, _OpenFlag, _ShareFlag, _Mode);
    va_end(args);
    return retval;
}

long __cdecl    tell(_In_ int _FileHandle)
{
    return UnixFD::get(_FileHandle)->tell();
}

int __cdecl     umask(_In_ int _Mode)
{
    return _umask(_Mode);
}

#endif
