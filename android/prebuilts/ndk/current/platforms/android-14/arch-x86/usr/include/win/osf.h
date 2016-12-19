
#ifndef _OSF_WIN32_H_
#define _OSF_WIN32_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

extern  int         win32_open_osfhandle(_In_ intptr_t _OSFileHandle, _In_ int _Flags, _In_ int _FileHandle);
extern  intptr_t    win32_release_osfhandle(_In_ int _FileHandle);
extern  intptr_t    win32_get_osfhandle(_In_ int _FileHandle);

extern  int         pagingfile(void);

__END_DECLS

#endif  /* _OSF_WIN32_H_ */
