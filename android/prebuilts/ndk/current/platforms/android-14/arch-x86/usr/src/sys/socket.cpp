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

#include "sys/socket.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win/unixfd.h"
#include <errno.h>
#include <cutils/log.h>

#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MSG_VALID_MASK (MSG_EOR | MSG_OOB | MSG_DONTROUTE)
#define MSG_ALIGN(len) ( ((len)+sizeof(size_t)-1) & ~(sizeof(size_t)-1) )
#define MSG_MAGICNO (0xac7d269b)

// If we do not read/send anything for 10 seconds, consider the connection broken.
// The other side is probably not working.  No point busy waiting or blocking
// beyond that.  The user probably kills the browser himself.
#define MSG_TIMEOUT 10

int socket(int address_family, int type, int protocol)
{
    return UnixFD::socket(address_family, type, protocol);
}

int bind(int socket, const struct sockaddr * address, int address_len)
{
    return FORWARD_CALL(BIND)((SOCKET)UnixFD::get(socket)->osHandle(), address, address_len);
}

int connect(int socket, const struct sockaddr * address, socklen_t address_len)
{
    return FORWARD_CALL(CONNECT)((SOCKET)UnixFD::get(socket)->osHandle(), address, address_len);
}

int listen(int socket, int backlog)
{
    return FORWARD_CALL(LISTEN)((SOCKET)UnixFD::get(socket)->osHandle(), backlog);
}

int accept(int socket, struct sockaddr *address, socklen_t *address_len)
{
    return UnixFD::get(socket)->accept(address, address_len);
}

int getsockname(int socket, struct sockaddr *address, socklen_t *address_len)
{
    return FORWARD_CALL(GETSOCKNAME)((SOCKET)UnixFD::get(socket)->osHandle(), address, address_len);
}

int getpeername(int socket, struct sockaddr *address, socklen_t *address_len)
{
    return FORWARD_CALL(GETPEERNAME)((SOCKET)UnixFD::get(socket)->osHandle(), address, address_len);
}

int socketpair(int domain, int type, int protocol, int * socket_vector)
{
    if (domain != AF_UNIX)
        return SOCKET_ERROR;

    if (type != SOCK_DGRAM && type != SOCK_STREAM)
        return SOCKET_ERROR;

    if (protocol != 0)
        return SOCKET_ERROR;

    if (UnixFD::socketpair(socket_vector, 0) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return 0;
}

int shutdown(int socket, int how)
{
    return FORWARD_CALL(SHUTDOWN)((SOCKET)UnixFD::get(socket)->osHandle(), how);
}

int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
{
    return FORWARD_CALL(SETSOCKOPT)((SOCKET)UnixFD::get(socket)->osHandle(), level, option_name, (const char*)option_value, option_len);
}

int getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len)
{
    return FORWARD_CALL(GETSOCKOPT)((SOCKET)UnixFD::get(socket)->osHandle(), level, option_name, (char*)option_value, option_len);
}

/* packed msg format
     magic_no(4) + name_len(4) + msg_flags(4) + ctrl_len(4) + data_len(4) + source_pid(4)
     name(align(msg_len)) + ctrl_data(ctrl_len) + iov(data_len) 

     support only one cmsg comsisting of FDs
*/

struct packed_msg {
    unsigned int magicno;
    unsigned int name_len;
    unsigned int ctrl_len;
    unsigned int data_len;
    unsigned int msg_flags;
    DWORD source_pid;
    unsigned int tot_len; // Total length of the message including this header, for more sanity checks
    unsigned int pad;     // Make the struct a multiple of 8B
};

struct packed_handle {
    void *handle;
    int fdtype;
};


static int get_sendmsg_size(const struct msghdr * message)
{
    size_t ret = MSG_ALIGN(sizeof(struct packed_msg));
    size_t iovsize = 0;

    ret += MSG_ALIGN(message->msg_namelen);
    if (message->msg_controllen > 0) {
        cmsghdr* cmsg_header = CMSG_FIRSTHDR(message);
        if (cmsg_header)
            ret += CMSG_SPACE(((cmsg_header->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(int)) * sizeof(struct packed_handle));
    }
    
    const iovec* src_msg_iov = message->msg_iov;
    for (size_t i=0; i<message->msg_iovlen; i++) {
        iovsize += src_msg_iov[i].iov_len;
    }
    ret += MSG_ALIGN(iovsize);

    if (ret > INT32_MAX) {
        ALOGE("msg is too big (%lld)\n", ret);
        return 0;
    }

    return ret;
}

/* 'timeout' is in seconds.  0 means poll (spin), and -1 means infinite.
 * If we do not write any bytes for timeout seconds, give up, set errno=ETIMEDOUT,
 * and return an error.
 * Returns the number of bytes written if successful, or -1 otherwise.
 */
static int blocking_writeall(int sock, char* buf, int buf_len, int timeout)
{
    char* ptr = buf;
    int written = 0;
    UnixFD* fd = UnixFD::get(sock);

    while (written < buf_len) {
        int s = fd->write(ptr, buf_len - written, 0);
        if (s > 0) {
            written += s;
            ptr += s;
        } else if (s < 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout != 0) {
                    fd_set wset;
                    struct timeval tv;
                    FD_ZERO(&wset);
                    FD_SET(sock, &wset);
                    if (timeout > 0) {
                        tv.tv_sec = timeout;
                        tv.tv_usec = 0;
                    }
                    int s = select(sock+1, NULL, &wset, NULL, timeout > 0 ? &tv : NULL);
                    if (s < 0)
                        break;
                    else if (s == 0) {
                        errno = ETIMEDOUT;
                        break;
                    } else {
                        // Go ahead and try again
                    }
                } else {
                    // Spin, try again without sleeping
                }
            } else {
                // Other errors probably mean the connection is broken
                break;
            }
        } else {
            // Not expected...
            break;
        }
    }

    return written == buf_len ? written : -1;
}

/* 'timeout' is in seconds.  0 means poll (no spin), and -1 means infinite.
 * If we do not read any bytes for timeout seconds, give up, set errno=ETIMEDOUT,
 * and return an error.
 * Returns the number of bytes read if successful, 0 if the connection is
 * gracefully closed, or -1 otherwise.
 */
static int blocking_readall(int sock, char* buf, int buf_len, int timeout)
{
    char* ptr = buf;
    int read = 0;
    UnixFD* fd = UnixFD::get(sock);

    while (read < buf_len) {
        int s = fd->read(ptr, buf_len - read, 0);
        if (s > 0) {
            ptr += s;
            read += s;
        } else if (s < 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout != 0) {
                    fd_set rset;
                    struct timeval tv;
                    FD_ZERO(&rset);
                    FD_SET(sock, &rset);
                    if (timeout > 0) {
                        tv.tv_sec = timeout;
                        tv.tv_usec = 0;
                    }
                    int s = select(sock+1, &rset, NULL, NULL, timeout > 0 ? &tv : NULL);
                    if (s < 0)
                        break;
                    else if (s == 0) {
                        errno = ETIMEDOUT;
                        break;
                    } else {
                        // Go ahead and try again
                    }
                }
            } else
                break;
        } else {
            // Connection closed
            return 0;
        }
    }

    return read == buf_len ? read : -1;
}

int sendmsg(int socket_descriptor, const struct msghdr * message, unsigned int flags)
{
    if (!message)
        return SOCKET_ERROR;

    if (flags & ~MSG_VALID_MASK)
        return SOCKET_ERROR;

    int msg_size = get_sendmsg_size(message);
    char *buf, *buf_ptr;

    if (msg_size <= 0)
        return SOCKET_ERROR;

    buf = (char *)malloc(msg_size);
    buf_ptr = buf;

    struct packed_msg *packedmsg = (struct packed_msg *)buf_ptr;

    packedmsg->magicno = MSG_MAGICNO;
    packedmsg->name_len = message->msg_namelen;
    packedmsg->msg_flags = message->msg_flags;
    packedmsg->ctrl_len = 0;
    packedmsg->data_len = 0;
    packedmsg->source_pid = ::GetCurrentProcessId();
    packedmsg->tot_len = msg_size;

    buf_ptr += MSG_ALIGN(sizeof(packed_msg));

    /* name */
    if (packedmsg->name_len > 0) {
        memcpy(buf_ptr, message->msg_name, packedmsg->name_len);
        buf_ptr += MSG_ALIGN(packedmsg->name_len);
    }

    /* control */
    if (message->msg_controllen > 0) {
        cmsghdr* cmsg_header = CMSG_FIRSTHDR(message);
        if (cmsg_header) {
            char *ctrlptr = buf_ptr;
            cmsghdr* packedcmsg_header = (cmsghdr *)ctrlptr;
            
            int fd_count = (cmsg_header->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(int);
            packedmsg->ctrl_len = CMSG_SPACE(fd_count * (MSG_ALIGN(sizeof(struct packed_handle))));
            packedcmsg_header->cmsg_level = cmsg_header->cmsg_level;
            packedcmsg_header->cmsg_type = cmsg_header->cmsg_type;
            packedcmsg_header->cmsg_len = cmsg_header->cmsg_len;
            ctrlptr += MSG_ALIGN(CMSG_ALIGN(sizeof(struct cmsghdr)));

            
            int *fdptr = (int *)CMSG_DATA(cmsg_header);
            for (int i=0; i<fd_count; i++) {
                if (fdptr[i] <= 0) {
                    ALOGE("invalid file desctiper to send (socket:%d fd:%d)\n", socket_descriptor, fdptr[i]);
                    packedcmsg_header->cmsg_len--;
                    continue;
                }
                struct packed_handle *hd = (struct packed_handle *)ctrlptr;
                int new_fd = UnixFD::get(fdptr[i])->dup();
                UnixFD* dup_fd = UnixFD::get(new_fd);
                hd->handle = dup_fd->osHandle();
                hd->fdtype = dup_fd->descriptorType();
                dup_fd->release();
                ctrlptr += MSG_ALIGN(sizeof(struct packed_handle));
            }       
            buf_ptr += packedmsg->ctrl_len;
        }
    }

    /* iov */    
    const iovec* src_msg_iov = message->msg_iov;
    for (size_t i = 0; i < message->msg_iovlen; ++i) {
        memcpy(buf_ptr, src_msg_iov[i].iov_base, src_msg_iov[i].iov_len);
        buf_ptr += src_msg_iov[i].iov_len;
        packedmsg->data_len += src_msg_iov[i].iov_len;
    }

    // Write the whole message
    int s = blocking_writeall(socket_descriptor, buf, msg_size, MSG_TIMEOUT);
    free(buf);
    return s;
}

int recvmsg(int socket_descriptor, struct msghdr * message, unsigned int flags)
{
    if (!message || !message->msg_iov || !message->msg_control)
        return SOCKET_ERROR;

    if (flags & ~MSG_VALID_MASK)
        return SOCKET_ERROR;

    UnixFD* socket = UnixFD::get(socket_descriptor);

    // read msg header
    struct packed_msg msghead;
    int readsize = sizeof(msghead);

    // Need to special-case the very first read, so do not use blocking_readall...
    // If we have not read bytes and see EWOULDBLOCK, we have to return that right away.
    int read = 0;
    char* buf_ptr = (char*)&msghead;
    while (read < readsize) {
        int s = socket->read(buf_ptr, readsize - read, 0);
        if (s > 0) {
            buf_ptr += s;
            read += s;
        } else if (s < 0) {
            if (errno == EWOULDBLOCK) {
                if (read == 0)
                    return SOCKET_ERROR;
            } else
                return SOCKET_ERROR;
        } else {
            // The caller (ConnectionUnix.cpp) tries to parse msg_control without
            // checking return code=0, so clear controllen...
            message->msg_controllen = 0;
            return 0; // Connection closed
        }
    }

    if (msghead.magicno != MSG_MAGICNO) {
        ALOGE("receive invalid message magic(%x) (socket:%d)\n", msghead.magicno, socket_descriptor);
        return SOCKET_ERROR;
    }

    // read name, control & data
    readsize = MSG_ALIGN(msghead.name_len) + MSG_ALIGN(msghead.ctrl_len) + MSG_ALIGN(msghead.data_len);
    if (readsize != msghead.tot_len - MSG_ALIGN(sizeof(struct packed_msg))) {
        ALOGE("invalid total length in the message: tot_len=%u expected=%u socket=%d\n",
              msghead.tot_len, readsize, socket_descriptor);
        return SOCKET_ERROR;
    }

    char *buf = NULL;
    if (readsize > 0) {
        buf = (char *)malloc(readsize);
        // Read the whole message
        int s = blocking_readall(socket_descriptor, buf, readsize, MSG_TIMEOUT);
        if (s != readsize) {
            free(buf);
            if (s == 0)
                message->msg_controllen = 0;
            return s;
        }
    }
    buf_ptr = buf;
    
    // copy name
    if (msghead.name_len > 0) {
        if (message->msg_name) {
            int msg_namelen = MIN(message->msg_namelen, msghead.name_len);
            message->msg_namelen = msg_namelen;
            memcpy(message->msg_name, buf_ptr, msg_namelen);
        }
        buf_ptr += MSG_ALIGN(msghead.name_len);
    }

    // compose controls
    if (message->msg_controllen > 0 && msghead.ctrl_len > 0) {
        char *ctrlptr = buf_ptr;
        cmsghdr *cmsg_header = CMSG_FIRSTHDR(message);
        cmsghdr *packedcmsg_header = (cmsghdr *)ctrlptr;

        cmsg_header->cmsg_level = packedcmsg_header->cmsg_level;
        cmsg_header->cmsg_type = packedcmsg_header->cmsg_type;
        cmsg_header->cmsg_len = packedcmsg_header->cmsg_len;

        ctrlptr += CMSG_ALIGN(sizeof(struct cmsghdr));
        int fd_count = (cmsg_header->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(int);
        int *fdprt = (int *)CMSG_DATA(cmsg_header);
        int res_count = 0;
        for (int i = 0; i < fd_count; i++) {
            struct packed_handle *hd = (struct packed_handle *)ctrlptr;
            int adopt_fd = UnixFD::adopt(msghead.source_pid, hd->handle, (UnixFD::Type)hd->fdtype);
            if (adopt_fd > 0) {
                fdprt[res_count++] = UnixFD::get(adopt_fd)->descriptorId();
            } else {
                ALOGE("receive wrong fd(%d) handle(%p) (socket:%d pid:%d)\n", adopt_fd, hd->handle, socket_descriptor, msghead.source_pid);
                cmsg_header->cmsg_len--;
            }
            ctrlptr += MSG_ALIGN(sizeof(struct packed_handle));
        }
        message->msg_controllen = CMSG_LEN(res_count*sizeof(int));
        buf_ptr += msghead.ctrl_len;
    } else {
        message->msg_controllen = 0;
    }
    
    // read iov
    int rest_len = msghead.data_len;
    if (message->msg_iov) {
        iovec* dst_msg_iov = message->msg_iov;
        for (size_t i = 0; i < message->msg_iovlen; ++i) {
            int iov_len = MIN(dst_msg_iov[i].iov_len, rest_len);
            if (iov_len > 0) {
                memcpy(dst_msg_iov[i].iov_base, buf_ptr, iov_len);
                buf_ptr += iov_len;
                rest_len -= iov_len;
            }
            dst_msg_iov[i].iov_len = iov_len;
        }
    }

    if (buf)
        free(buf);

    return msghead.data_len;
}

ssize_t send(int socket, const void *message, size_t length, unsigned int flags)
{
    return UnixFD::get(socket)->write(message, length, flags);
}

ssize_t recv(int socket, void *buffer, size_t length, unsigned int flags)
{
    return UnixFD::get(socket)->read(buffer, length, flags);
}

ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    return FORWARD_CALL(SENDTO)((SOCKET)UnixFD::get(socket)->osHandle(), (const char*)message, length, flags, dest_addr, dest_len);
}

ssize_t recvfrom(int socket, void *buffer, size_t length, unsigned int flags, const struct sockaddr *address, socklen_t *address_len)
{
    return FORWARD_CALL(RECVFROM)((SOCKET)UnixFD::get(socket)->osHandle(), (char*)buffer, length, flags, (struct sockaddr *)address, address_len);
}

#endif
