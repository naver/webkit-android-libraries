/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 * Copyright (C) 2013 Naver Corp.
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
#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <sys/cdefs.h>

#include <direct.h>
#include <stdio.h>
#undef  __STDC__
#define __STDC__ 1
#include <io.h>
#undef  __STDC__
#include <process.h>
#include <shlwapi.h>
#include <sys/stat.h>

#define PATH_MAX MAX_PATH

#undef lseek

__BEGIN_DECLS

#define O_CREAT _O_CREAT
#define O_EXCL  _O_EXCL
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define S_IRGRP _S_IREAD
#define S_IWGRP _S_IWRITE
#define S_IXUSR 0
#define S_IXGRP 0
#define F_OK    0

#define S_ISDIR(m) ((m) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)

#undef  mkdir
#define mkdir(a, b)     _mkdir(a)

extern pid_t    gettid(void);

/* Macros for access() */
#define R_OK    4  /* Read */
#define W_OK    2  /* Write */
#define X_OK    1  /* Execute */
#define F_OK    0  /* Existence */

extern int      access(const char *, int);
extern int      pipe(int *);

extern int      close(int);
extern off_t    lseek(int, off_t, int);

extern int      read(int, void *, size_t);
extern int      write(int, const void *, size_t);

extern int      dup(int);
extern int      dup2(int, int);
extern int      ioctl(int, int, ssize_t*);
extern int      ftruncate(int, off_t);

extern unsigned sleep(unsigned);

extern int      gethostname(char *, size_t);

extern long     getpagesize (void);

extern int      isatty(int);

/* stdlib.h */
extern int      mkstemp (char *);

/* links to windows-specific c-style functions */
int __cdecl     chmod(const char * _Filename, int _AccessMode);
int __cdecl     chsize(int _FileHandle, long _Size);
int __cdecl     eof(int _FileHandle);
long __cdecl    filelength(int _FileHandle);
int __cdecl     locking(int _FileHandle, int _LockMode, long _NumOfBytes);
char * __cdecl  mktemp(char * _TemplateName);
int __cdecl     setmode(int _FileHandle, int _Mode);
int __cdecl     sopen(const char * _Filename, int _OpenFlag, int _ShareFlag, ...);
long __cdecl    tell(int _FileHandle);
int __cdecl     umask(int _Mode);

__END_DECLS

#include <win/win32_deprecated.h>

#endif /* _UNISTD_H_ */
