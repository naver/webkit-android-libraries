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

#include "sys/select.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win/unixfd.h"
#include "win/timer_resolution_controller.h"

// Derived from code from Windows winsock.h
void FD_CLR(int fd, fd_set * set)
{
    u_int __i;
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) {
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) {
            while (__i < ((fd_set FAR *)(set))->fd_count-1) {
                ((fd_set FAR *)(set))->fd_array[__i] =
                    ((fd_set FAR *)(set))->fd_array[__i+1];
                __i++;
            }
            ((fd_set FAR *)(set))->fd_count--;
        }
    }
}

void FD_SET(int fd, fd_set * set)
{
    u_int __i;
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) {
        if (((fd_set FAR *)(set))->fd_array[__i] == (fd)) {
            break;
        }
    }
    if (__i == ((fd_set FAR *)(set))->fd_count) {
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) {
            ((fd_set FAR *)(set))->fd_array[__i] = (fd);
            ((fd_set FAR *)(set))->fd_count++;
        }
    }
}

void FD_ZERO(fd_set * set)
{
    (((fd_set FAR *)(set))->fd_count=0);
}

int  FD_ISSET(int fd, fd_set * set)
{
    return __WSAFDIsSet((SOCKET)UnixFD::get(fd)->osHandle(), (fd_set FAR *)(set)) != 0;
}

static fd_set * convert_fd_set(fd_set * set)
{
    if (!set)
        return 0;

    for (u_int i = 0; i < set->fd_count; ++i) {
        UnixFD* fd = UnixFD::get(set->fd_array[i]);
        ASSERT(fd->descriptorType() == UnixFD::Socket);
        set->fd_array[i] = (SOCKET)fd->osHandle();
    }
    return set;
}

int select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
    TimerResolutionController controller(timeout);
    return FORWARD_CALL(SELECT)(nfds, convert_fd_set(readfds), convert_fd_set(writefds), convert_fd_set(exceptfds), timeout);
}

#endif
