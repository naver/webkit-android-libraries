/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H

#include <asm/types.h>
#include <linux/uio.h>

#undef CMSG_DATA

struct msghdr {
 void * msg_name;
 int msg_namelen;
 struct iovec * msg_iov;
 __kernel_size_t msg_iovlen;
 void * msg_control;
 __kernel_size_t msg_controllen;
 unsigned msg_flags;
};

#ifndef _WSACMSGHDR
struct cmsghdr {
 __kernel_size_t cmsg_len;
 int cmsg_level;
 int cmsg_type;
};
#endif

#undef CMSG_NXTHDR
#undef CMSG_SPACE
#undef CMSG_LEN
#undef CMSG_FIRSTHDR

#define __CMSG_NXTHDR(ctl, len, cmsg) __cmsg_nxthdr((ctl),(len),(cmsg))
#define CMSG_NXTHDR(mhdr, cmsg) cmsg_nxthdr((mhdr), (cmsg))

#define CMSG_ALIGN(len) ( ((len)+sizeof(size_t)-1) & ~(sizeof(size_t)-1) )

#define CMSG_DATA(cmsg) ((void *)((char *)(cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr))))
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))

#define __CMSG_FIRSTHDR(ctl,len) ((len) >= sizeof(struct cmsghdr) ?   (struct cmsghdr *)(ctl) :   (struct cmsghdr *)NULL)
#define CMSG_FIRSTHDR(msg) __CMSG_FIRSTHDR((msg)->msg_control, (msg)->msg_controllen)
#define CMSG_OK(mhdr, cmsg) ((cmsg)->cmsg_len >= sizeof(struct cmsghdr) &&   (cmsg)->cmsg_len <= (unsigned long)   ((mhdr)->msg_controllen -   ((char *)(cmsg) - (char *)(mhdr)->msg_control)))

#ifdef __GNUC__
#define __KINLINE static __inline__
#elif defined(__cplusplus)
#define __KINLINE static inline
#else
#define __KINLINE static
#endif

__KINLINE struct cmsghdr * __cmsg_nxthdr(void *__ctl, __kernel_size_t __size,
 struct cmsghdr *__cmsg)
{
 struct cmsghdr * __ptr;

 __ptr = (struct cmsghdr*)(((unsigned char *) __cmsg) + CMSG_ALIGN(__cmsg->cmsg_len));
 if ((unsigned long)((char*)(__ptr+1) - (char *) __ctl) > __size)
 return (struct cmsghdr *)0;

 return __ptr;
}

__KINLINE struct cmsghdr * cmsg_nxthdr (struct msghdr *__msg, struct cmsghdr *__cmsg)
{
 return __cmsg_nxthdr(__msg->msg_control, __msg->msg_controllen, __cmsg);
}

#define SCM_RIGHTS 0x01  
#define SCM_CREDENTIALS 0x02  
#define SCM_SECURITY 0x03  

#define AF_UNIX 1  
#define AF_LOCAL 1  


#define MSG_EOR 0x80  

#endif
