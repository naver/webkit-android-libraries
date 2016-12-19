/*
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

#include "mman.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win/unixfd.h"
#include <windows.h>
#include <errno.h>
#include <io.h>

#define PAGING_FILE_HANDLE_VALUE -3

static int _mman_error(DWORD err, int deferr)
{
    switch (err)
    {
    case 0:
        return 0;
    default:
        break;
    }
    return err;
}

static DWORD _mman_prot_page(int prot)
{
    if (prot == PROT_NONE)
        return 0;

    DWORD protect = 0;
    if ((prot & PROT_EXEC) != 0)
        protect = ((prot & PROT_WRITE) != 0) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    else
        protect = ((prot & PROT_WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
    return protect;
}

static DWORD _mman_prot_flie(int prot)
{
    if (prot == PROT_NONE)
        return 0;

    DWORD desiredAccess = 0;
    if ((prot & PROT_READ) != 0)
        desiredAccess |= FILE_MAP_READ;
    if ((prot & PROT_WRITE) != 0)
        desiredAccess |= FILE_MAP_WRITE;
    if ((prot & PROT_EXEC) != 0)
        desiredAccess |= FILE_MAP_EXECUTE;
    return desiredAccess;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    errno = 0;

    bool unsupported = (flags & MAP_FIXED) != 0 || prot == PROT_EXEC;
    if (len == 0 || unsupported)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }

    UnixFD* fd = UnixFD::get(fildes);
    UnixFD::Type fdtype = fd->descriptorType();
    HANDLE h = (HANDLE)fd->osHandle();

    if (h == INVALID_HANDLE_VALUE && fdtype == UnixFD::File)
    {
        errno = EBADF;
        return MAP_FAILED;
    }

    bool pagingfile = (fdtype == UnixFD::PagingFile) && (h != INVALID_HANDLE_VALUE);

    off_t maxSize = off + (off_t)len;
    DWORD protect = _mman_prot_page(prot);

    HANDLE fm = (pagingfile) ? h : CreateFileMapping(h, NULL, protect, 0, maxSize, NULL);

    if (fm == NULL)
    {
        errno = _mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    void * map = MapViewOfFile(fm, _mman_prot_flie(prot), 0, off, len);

    if (fdtype == UnixFD::File)
        CloseHandle(fm);
    else if (!pagingfile)
        fd->setOSHandle(fm);

    if (map == NULL)
    {
        errno = _mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    return map;
}

int munmap(void *addr, size_t len)
{
    if (UnmapViewOfFile(addr))
        return 0;

    errno =  _mman_error(GetLastError(), EPERM);

    return -1;
}

#endif
