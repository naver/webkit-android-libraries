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
#ifndef _UNIXFD_H_
#define _UNIXFD_H_

#include <unistd.h>

class UnixFD {
public:
    enum Type {
        Unknown,
        File,
        Socket,
        PagingFile
    };

    static int open(const char * filename, int openFlag, int permissionMode);
    static int sopen(const char * filename, int openFlag, int shareFlag, int permissionMode);
    static int socket(int address_family, int type, int protocol);
    static int socketpair(int* fds, int make_overlapped);

    static UnixFD* create(Type);
    static UnixFD* get(int);

    static int adopt(void* oshandle, Type type);
    static int adopt(void* oshandle, Type type, int fd);
    static int adopt(pid_t source_pid, void* oshandle, Type type);

    int accept(struct sockaddr * addr, int * addrlen);

    int close();
    void* release();

    long tell();
    off_t lseek(off_t, int);

    int read(void *, size_t, unsigned int);
    int write(const void *, size_t, unsigned int);
    int eof();

    int dup();
    int dup2(int);

    int isatty();

    int fcntl(int command, int flags);

    int chsize(long _Size);
    long filelength();

    int locking(int _LockMode, long _NumOfBytes);
    int setmode(int _Mode);

    void* osHandle() const { return m_osHandle; }
    void setOSHandle(void* handle) { m_osHandle = handle; }
    int descriptorId() const { return m_descriptorId; }

    Type descriptorType() const { return m_type; }
    bool isSocket() const { return m_type == Socket; }

private:
    UnixFD(void*, int, int, Type);
    ~UnixFD();

    int osfd();

    void* m_osHandle;
    int m_osfd;
    int m_descriptorId;
    Type m_type;
    int m_flags;
};

#endif /* _UNIXFD_H_ */
