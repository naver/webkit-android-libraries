/*
 * Copyright (C) 2014 Naver Corp. All rights reserved.
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
#include "ws2_shims.h"

#if defined(WIN32) || defined(_WINDOWS)

#include "win/unixfd.h"

extern "C" {

int     WSAAPI  closesocket(SOCKET s)
{
    return FORWARD_CALL(CLOSESOCKET)((SOCKET)UnixFD::get(s)->osHandle());
}

int     WSAAPI  ioctlsocket(SOCKET s, long cmd, _Inout_ u_long FAR* argp)
{
    return FORWARD_CALL(IOCTLSOCKET)((SOCKET)UnixFD::get(s)->osHandle(), cmd, argp);
}

u_long  WSAAPI  htonl(u_long hostlong)
{
    return FORWARD_CALL(HTONL)(hostlong);
}

u_short WSAAPI  htons(u_short hostshort)
{
    return FORWARD_CALL(HTONS)(hostshort);
}

u_long  WSAAPI  ntohl(u_long netlong)
{
    return FORWARD_CALL(NTOHL)(netlong);
}

u_short WSAAPI  ntohs(u_short netshort)
{
    return FORWARD_CALL(NTOHS)(netshort);
}

int     WSAAPI  WSAStartup(WORD wVersionRequested, _Out_ LPWSADATA lpWSAData)
{
    return WinSock2WSAStartup(wVersionRequested, lpWSAData);
}

int     WSAAPI  WSACleanup(void)
{
    return FORWARD_CALL(WSACLEANUP)();
}

void    WSAAPI  WSASetLastError(int iError)
{
    FORWARD_CALL(WSASETLASTERROR)(iError);
}

int     WSAAPI  WSAGetLastError(void)
{
    return FORWARD_CALL(WSAGETLASTERROR)();
}

int     WSAAPI  WSAIoctl(SOCKET s, DWORD dwIoControlCode, _In_reads_bytes_opt_(cbInBuffer) LPVOID lpvInBuffer, DWORD cbInBuffer,
    _Out_writes_bytes_to_opt_(cbOutBuffer, *lpcbBytesReturned) LPVOID lpvOutBuffer, DWORD cbOutBuffer, _Out_ LPDWORD lpcbBytesReturned,
    _Inout_opt_ LPWSAOVERLAPPED lpOverlapped, _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    return FORWARD_CALL(WSAIOCTL)((SOCKET)UnixFD::get(s)->osHandle(), dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);
}

int PASCAL FAR  __WSAFDIsSet(SOCKET fd, fd_set FAR * set)
{
    return FORWARD_CALL(__WSAFDISSET)(fd, set);
}

INT     WSAAPI  getaddrinfo(_In_opt_ PCSTR pNodeName, _In_opt_ PCSTR pServiceName, _In_opt_ const ADDRINFOA * pHints, _Outptr_ PADDRINFOA * ppResult)
{
    return FORWARD_CALL(GETADDRINFO)(pNodeName, pServiceName, pHints, ppResult);
}

VOID    WSAAPI  freeaddrinfo(_In_opt_ PADDRINFOA pAddrInfo)
{
    return FORWARD_CALL(FREEADDRINFO)(pAddrInfo);
}

}

#endif
