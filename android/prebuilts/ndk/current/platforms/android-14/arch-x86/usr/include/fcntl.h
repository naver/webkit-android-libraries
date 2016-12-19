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

#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/cdefs.h>

#if defined(_MSC_VER)
#undef  __STDC__
#include <../ucrt/fcntl.h>
#define __STDC__ 1
#include <../ucrt/io.h>
#undef  __STDC__
#endif

#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */

typedef	int	mode_t;	/* permissions */

__BEGIN_DECLS

#ifndef O_ASYNC
#define O_ASYNC  FASYNC
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC  02000000
#endif

// from asm-generic/fcntl.h
#ifndef O_NONBLOCK
#define O_NONBLOCK 00004000
#endif
#define F_DUPFD 0  
#define F_GETFD 1  
#define F_SETFD 2  
#define F_GETFL 3  
#define F_SETFL 4  
#define FD_CLOEXEC 1  

extern int  open(const char*  path, int  mode, ...);
extern int  fcntl(int   fd, int   command, ...);
extern int  creat(const char*  path, mode_t  mode);

__END_DECLS

#endif /* _FCNTL_H */
