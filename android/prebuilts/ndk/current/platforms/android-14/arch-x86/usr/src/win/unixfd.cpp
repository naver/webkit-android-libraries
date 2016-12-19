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
#include "unixfd.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "fcntl.h"
#include "osf.h"
#include "sys/select.h"
#include <errno.h>
#include <map>
#include <mutex>
#include <cutils/log.h>

#undef _get_osfhandle
#undef _open_osfhandle

static int fds_hash_function(void* key)
{
    int key_value = (int)key;
    return key_value % 127;
}

static bool fds_key_equals(void* keyA, void* keyB)
{
    return keyA == keyB;
}

static std::map<int, UnixFD*>& fds_map()
{
    static std::map<int, UnixFD*> map;
    return map;
}

static std::mutex& fds_mutex()
{
    static std::mutex mutex;
    return mutex;
}

static int getDescriptorId()
{
    static int id = 0;
    while (fds_map().count(++id)) { }
    return id;
}

UnixFD::UnixFD(void* oshandle, int fd, int id, Type type)
    : m_osHandle(oshandle)
    , m_osfd((type == File && fd > 0) ? fd : _open_osfhandle((intptr_t)m_osHandle, _O_BINARY))
    , m_descriptorId(id)
    , m_type(type)
    , m_flags(0)
{
}

UnixFD::~UnixFD()
{
    close();
}

int UnixFD::osfd()
{
    ASSERT(m_type == File);
    return m_osfd;
}

int UnixFD::open(const char * filename, int openFlag, int permissionMode)
{
    std::lock_guard<std::mutex> locker(fds_mutex());

    int filehandle = _open(filename, openFlag, permissionMode);
    if (filehandle == -1)
        return -1;

    HANDLE osfhandle = (HANDLE)_get_osfhandle(filehandle);
    if (osfhandle == INVALID_HANDLE_VALUE)
        return -1;

    UnixFD* unixfd = new UnixFD(osfhandle, filehandle, getDescriptorId(), File);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

int UnixFD::sopen(const char * filename, int openFlag, int shareFlag, int permissionMode)
{
    std::lock_guard<std::mutex> locker(fds_mutex());

    int filehandle = _sopen(filename, openFlag, shareFlag, permissionMode);
    if (filehandle == -1)
        return -1;

    HANDLE osfhandle = (HANDLE)_get_osfhandle(filehandle);
    if (osfhandle == INVALID_HANDLE_VALUE)
        return -1;

    UnixFD* unixfd = new UnixFD(osfhandle, filehandle, getDescriptorId(), File);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

class WSAHandle {
public:
    WSAHandle();
    ~WSAHandle();

    bool startup(WORD versionRequired);

private:
    WSADATA m_wsadata;
    bool m_started;
};

WSAHandle::WSAHandle()
    : m_started(false)
{
    memset(&m_wsadata, 0, sizeof(WSADATA));
}

WSAHandle::~WSAHandle()
{
    FORWARD_CALL(WSACLEANUP)();
}

bool WSAHandle::startup(WORD versionRequired)
{
    if (m_started)
        return true;

    m_started = WSAStartup(versionRequired, &m_wsadata) == 0;
    return m_started;
}

static int startup(WORD versionRequired)
{
    static WSAHandle wsa;

    if (!wsa.startup(versionRequired)) {
        ALOGE("WSAStartup failed with error: %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

int UnixFD::socket(int address_family, int type, int protocol)
{
    std::lock_guard<std::mutex> locker(fds_mutex());

    if (startup(MAKEWORD(2, 2)) == -1)
        return -1;

    SOCKET sock = FORWARD_CALL(SOCKET)(address_family, type, protocol);
    if (sock == -1)
        return -1;

    UnixFD* unixfd = new UnixFD((void*)sock, -1, getDescriptorId(), Socket);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

static void handle_wsa_socket_error()
{
    switch (WSAGetLastError()) {
    case WSAEWOULDBLOCK:
        errno = EWOULDBLOCK;
        break;
    case WSAEINTR:
        errno = EINTR;
        break;
    case WSAEINVAL:
        errno = EINVAL;
        break;
    case WSAECONNRESET:
        errno = ECONNRESET;
        break;
    case WSAECONNABORTED:
        errno = ECONNABORTED;
        break;
    default:
        errno = EBADF;
        break;
    }
}

extern "C" int dumb_socketpair(SOCKET socks[2], int make_overlapped); // socketpair.c

int UnixFD::socketpair(int* fds, int make_overlapped)
{
    if (startup(MAKEWORD(2, 2)) == -1)
        return -1;

    SOCKET sockets[2];
    if (dumb_socketpair(sockets, make_overlapped) == SOCKET_ERROR) {
        handle_wsa_socket_error();
        return -1;
    }

    fds[0] = UnixFD::adopt(-1, (void*)sockets[0], UnixFD::Socket);
    fds[1] = UnixFD::adopt(-1, (void*)sockets[1], UnixFD::Socket);
    return 0;
}

UnixFD* UnixFD::create(Type type)
{
    std::lock_guard<std::mutex> locker(fds_mutex());
    UnixFD* unixfd = new UnixFD(INVALID_HANDLE_VALUE, 0, getDescriptorId(), type);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd;
}

UnixFD* UnixFD::get(int fd)
{
    std::lock_guard<std::mutex> locker(fds_mutex());
    return fds_map()[fd];
}

int UnixFD::adopt(void* oshandle, Type type)
{
    if (!oshandle)
        return -1;

    HANDLE os_handle = (HANDLE)oshandle;
    if (os_handle == INVALID_HANDLE_VALUE)
        return -1;

    std::lock_guard<std::mutex> locker(fds_mutex());

    UnixFD* unixfd = new UnixFD(os_handle, -1, getDescriptorId(), type);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

int UnixFD::adopt(void* oshandle, Type type, int fd)
{
    if (!oshandle)
        return -1;

    HANDLE os_handle = (HANDLE)oshandle;
    if (os_handle == INVALID_HANDLE_VALUE)
        return -1;

    std::lock_guard<std::mutex> locker(fds_mutex());

    if (fd == -1)
        fd = getDescriptorId();

    assert(fds_map().count(fd) == 0);
    UnixFD* unixfd = new UnixFD(os_handle, -1, fd, type);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

int UnixFD::adopt(pid_t source_pid, void* oshandle, Type type)
{
    HANDLE source_process = (source_pid == -1) ? ::GetCurrentProcess() : ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, source_pid);
    if (source_pid != -1 && source_process == INVALID_HANDLE_VALUE)
        return -1;

    HANDLE duplicated_handle;
    if (source_pid == -1)
        duplicated_handle = oshandle;
    else {
        // Copy the handle into our process and close the handle that the sending process created for us.
        BOOL success = ::DuplicateHandle(source_process, oshandle, ::GetCurrentProcess(), &duplicated_handle, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
        ASSERT(success);
    }

    ::CloseHandle(source_process);
    return adopt(duplicated_handle, type);
}

int UnixFD::accept(struct sockaddr * addr, int * addrlen)
{
    if (!isSocket())
        return -1;

    SOCKET sock = FORWARD_CALL(ACCEPT)((SOCKET)m_osHandle, addr, addrlen);
    if (sock == -1)
        return -1;

    std::lock_guard<std::mutex> locker(fds_mutex());

    UnixFD* unixfd = new UnixFD((void*)sock, -1, getDescriptorId(), Socket);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

int UnixFD::close()
{
    if (!m_osHandle)
        return 0;

    int result = 0;
    switch (m_type) {
    case File:
    case PagingFile:
        if (!::CloseHandle(m_osHandle))
            result = -1;
        break;
    case Socket:
        result = FORWARD_CALL(CLOSESOCKET)((SOCKET)m_osHandle);
        break;
    }

    release();

    return result;
}

void* UnixFD::release()
{
    if (!m_osHandle)
        return nullptr;

    std::lock_guard<std::mutex> locker(fds_mutex());

    fds_map().erase(descriptorId());
    void* osHandle = m_osHandle;
    m_osHandle = 0;
    m_descriptorId = 0;
    m_type = Unknown;
    delete this;

    return osHandle;
}

long UnixFD::tell()
{
    return _tell(osfd());
}

off_t UnixFD::lseek(off_t offset, int whence)
{
    return _lseek(osfd(), offset, whence);
}

int UnixFD::read(void * buf, size_t nbyte, unsigned int flags)
{
    if (!m_osHandle)
        return 0;

    int result = 0;
    DWORD bytesRead = 0;
    switch (m_type) {
    case File:
        if (!::ReadFile(m_osHandle, buf, nbyte, &bytesRead, 0))
            result = -1;
        result = bytesRead;
        break;
    case Socket:
        result = FORWARD_CALL(RECV)((SOCKET)m_osHandle, (char*) buf, nbyte, flags);
        if (result < 0) {
            handle_wsa_socket_error();
        }
        break;
    }

    return result;
}

int UnixFD::write(const void * buf, size_t count, unsigned int flags)
{
    if (!m_osHandle)
        return 0;

    int result = 0;
    DWORD bytesWritten = 0;
    switch (m_type) {
    case File:
        if (!::WriteFile(m_osHandle, buf, count, &bytesWritten, 0))
            result = -1;
        result = bytesWritten;
        break;
    case Socket:
        result = FORWARD_CALL(SEND)((SOCKET)m_osHandle, (char*) buf, count, flags);
        if (result < 0) {
            handle_wsa_socket_error();
        }
        break;
    }

    return result;
}

int UnixFD::eof()
{
    return _eof(osfd());
}

int UnixFD::dup()
{
    if (!m_osHandle)
        return 0;

    HANDLE duplicated;
    if (!::DuplicateHandle(::GetCurrentProcess(), m_osHandle, ::GetCurrentProcess(), &duplicated, 0, FALSE, DUPLICATE_SAME_ACCESS))
        return -1;

    if (duplicated == INVALID_HANDLE_VALUE)
        return -1;

    std::lock_guard<std::mutex> locker(fds_mutex());

    UnixFD* unixfd = new UnixFD(duplicated, -1, getDescriptorId(), m_type);
    fds_map()[unixfd->descriptorId()] = unixfd;
    return unixfd->descriptorId();
}

int UnixFD::dup2(int newfd)
{
    ASSERT(0);
    return _dup2(osfd(), newfd);
}

int UnixFD::isatty()
{
    return _isatty(osfd());
}

int UnixFD::fcntl(int command, int flags)
{
    switch (command) {
    case F_GETFD:
        DWORD handleFlags;
        ::GetHandleInformation(m_osHandle, &handleFlags);
        return (handleFlags & HANDLE_FLAG_INHERIT) == 0 ? 0 : FD_CLOEXEC;
        break;
    case F_SETFD:
        if (flags & FD_CLOEXEC)
            ::SetHandleInformation(m_osHandle, HANDLE_FLAG_INHERIT, 0);
        else
            ::SetHandleInformation(m_osHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        return 0;
        break;
    case F_GETFL:
        if (flags & O_NONBLOCK)
            return m_flags & O_NONBLOCK;
        return 0;
    case F_SETFL:
        if (flags & O_NONBLOCK && !(m_flags & O_NONBLOCK)) {
            u_long iMode = 1;
            if (FORWARD_CALL(IOCTLSOCKET)((SOCKET)m_osHandle, FIONBIO, &iMode) == SOCKET_ERROR) {
                handle_wsa_socket_error();
                return -1;
            }
            m_flags |= O_NONBLOCK;
        }
        return 0;
    }
    errno = EBADF;
    return -1;
}

int UnixFD::chsize(long size)
{
    return _chsize(osfd(), size);
}

long UnixFD::filelength()
{
    if (!m_osHandle)
        return 0;

    u_long length = 0;
    switch (m_type) {
    case File:
        return _filelength(osfd());
    case Socket:
        if (FORWARD_CALL(IOCTLSOCKET)((SOCKET)m_osHandle, FIONREAD, &length) == SOCKET_ERROR) {
            handle_wsa_socket_error();
            break;
        }
        return length;
    }

    return -1;
}

int UnixFD::locking(int lockMode, long numOfBytes)
{
    return _locking(osfd(), lockMode, numOfBytes);
}

int UnixFD::setmode(int mode)
{
    return _setmode(osfd(), mode);
}

#endif
