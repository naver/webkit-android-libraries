#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################
# cURL/libcurl CMake script
# by Tetetest and Sukender (Benoit Neil)

# TODO:
# The output .so file lacks the soname number which we currently have within the lib/Makefile.am file
# Add full (4 or 5 libs) SSL support
# Add INSTALL target (EXTRA_DIST variables in Makefile.am may be moved to Makefile.inc so that CMake/CPack is aware of what's to include).
# Add CTests(?)
# Check on all possible platforms
# Test with as many configurations possible (With or without any option)
# Create scripts that help keeping the CMake build system up to date (to reduce maintenance). According to Tetetest:
#  - lists of headers that 'configure' checks for;
#  - curl-specific tests (the ones that are in m4/curl-*.m4 files);
#  - (most obvious thing:) curl version numbers.
# Add documentation subproject
#
# To check:
# (From Daniel Stenberg) The cmake build selected to run gcc with -fPIC on my box while the plain configure script did not.
# (From Daniel Stenberg) The gcc command line use neither -g nor any -O options. As a developer, I also treasure our configure scripts's --enable-debug option that sets a long range of "picky" compiler options.
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set_property(DIRECTORY . PROPERTY FOLDER "cURL")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")
include(Utilities)
include(Macros)

project( CURL C )

message(WARNING "the curl cmake build system is poorly maintained. Be aware")

file (READ ${CURL_SOURCE_DIR}/include/curl/curlver.h CURL_VERSION_H_CONTENTS)
string (REGEX MATCH "#define LIBCURL_VERSION \"[^\"]*"
  CURL_VERSION ${CURL_VERSION_H_CONTENTS})
string (REGEX REPLACE "[^\"]+\"" "" CURL_VERSION ${CURL_VERSION})
string (REGEX MATCH "#define LIBCURL_VERSION_NUM 0x[0-9a-fA-F]+"
  CURL_VERSION_NUM ${CURL_VERSION_H_CONTENTS})
string (REGEX REPLACE "[^0]+0x" "" CURL_VERSION_NUM ${CURL_VERSION_NUM})

include_regular_expression("^.*$")    # Sukender: Is it necessary?

# Setup package meta-data
# SET(PACKAGE "curl")
message(STATUS "curl version=[${CURL_VERSION}]")
# SET(PACKAGE_TARNAME "curl")
# SET(PACKAGE_NAME "curl")
# SET(PACKAGE_VERSION "-")
# SET(PACKAGE_STRING "curl-")
# SET(PACKAGE_BUGREPORT "a suitable curl mailing list => https://curl.haxx.se/mail/")
set(OPERATING_SYSTEM "${CMAKE_SYSTEM_NAME}")
set(OS "\"${CMAKE_SYSTEM_NAME}\"")

include_directories(${PROJECT_BINARY_DIR}/include/curl)
include_directories( ${CURL_SOURCE_DIR}/include )

if (WIN32)
  set(YES_WIN32 TRUE)
  set(NO_WIN32 FALSE)
else ()
  set(YES_WIN32 FALSE)
  set(NO_WIN32 TRUE)
endif ()

set(BUILD_CURL_EXE      ${YES_WIN32})
set(BUILD_CURL_TESTS    NO)
set(CURL_STATICLIB      YES)

set(ENABLE_DEBUG        NO)
set(ENABLE_CURLDEBUG    NO)

if (ENABLE_DEBUG)
  # DEBUGBUILD will be defined only for Debug builds
  if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:DEBUGBUILD>)
  else()
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUGBUILD)
  endif()
  set(ENABLE_CURLDEBUG ON)
endif()

if (ENABLE_CURLDEBUG)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS CURLDEBUG)
endif()

# initialize CURL_LIBS
set(CURL_LIBS "")

# (WebKit) Removed configuration calculations
set(BUILDING_LIBCURL            YES)

set(CURL_DISABLE_COOKIES        NO)
set(CURL_DISABLE_CRYPTO_AUTH    NO)
set(CURL_DISABLE_DICT           YES)
set(CURL_DISABLE_FILE           NO)
set(CURL_DISABLE_FTP            NO)
set(CURL_DISABLE_GOPHER         YES)
set(CURL_DISABLE_IMAP           YES)
set(CURL_DISABLE_HTTP           NO)
set(CURL_DISABLE_LDAP           YES)
set(CURL_DISABLE_LDAPS          YES)
set(CURL_DISABLE_POP3           YES)
set(CURL_DISABLE_PROXY          NO)
set(CURL_DISABLE_RTSP           YES)
set(CURL_DISABLE_RTMP           YES)
set(CURL_DISABLE_SMB            YES)
set(CURL_DISABLE_SMTP           YES)
set(CURL_DISABLE_TELNET         YES)
set(CURL_DISABLE_TFTP           YES)
set(CURL_DISABLE_VERBOSE_STRINGS NO)

set(ENABLE_IPV6 YES)
if(ENABLE_IPV6 AND NOT WIN32)
  include(CheckStructHasMember)
  check_struct_has_member("struct sockaddr_in6" sin6_addr "netinet/in.h"
                          HAVE_SOCKADDR_IN6_SIN6_ADDR)
  check_struct_has_member("struct sockaddr_in6" sin6_scope_id "netinet/in.h"
                          HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID)
  if(NOT HAVE_SOCKADDR_IN6_SIN6_ADDR)
    message(WARNING "struct sockaddr_in6 not available, disabling IPv6 support")
    # Force the feature off as this name is used as guard macro...
    set(ENABLE_IPV6 OFF
        CACHE BOOL "Define if you want to enable IPv6 support" FORCE)
  endif()
endif()

# We need ansi c-flags, especially on HP
set(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")
set(CMAKE_REQUIRED_FLAGS ${CMAKE_ANSI_CFLAGS})

# Include all the necessary files for macros
include (CheckTypeSize)

# On windows preload settings
if(WIN32)
  set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} -D_WINSOCKAPI_")
  include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Platforms/WindowsCache.cmake)
endif(WIN32)

set(HAVE_ARPA_INET_H        YES)
set(HAVE_ASSERT_H           YES)
set(HAVE_BOOL_T             YES)
set(HAVE_CRYPTO_CLEANUP_ALL_EX_DATA YES)
set(HAVE_DLFCN_H            YES)
set(HAVE_ENGINE_LOAD_BUILTIN_ENGINES YES)
set(HAVE_ERRNO_H            YES)
set(HAVE_FCNTL              YES)
set(HAVE_FCNTL_H            YES)
set(HAVE_FCNTL_O_NONBLOCK   YES)
set(HAVE_FREEADDRINFO       YES)
set(HAVE_GETADDRINFO        YES)
set(HAVE_GETHOSTBYADDR      YES)
set(HAVE_GETHOSTBYNAME      YES)
set(HAVE_GETHOSTNAME        YES)
set(HAVE_GETNAMEINFO        YES)
set(HAVE_GETPROTOBYNAME     YES)
set(HAVE_GETTIMEOFDAY       YES)
set(HAVE_GMTIME_R           ${NO_WIN32})
set(HAVE_INET_ADDR          YES)
set(HAVE_INET_NTOP          YES)
set(HAVE_INET_PTON          YES)
set(HAVE_INTTYPES_H         YES)
set(HAVE_IOCTL              YES)
set(HAVE_IOCTL_FIONBIO      YES)
set(HAVE_IO_H               YES)

set(HAVE_LIBGEN_H           YES)
set(HAVE_LIBSSL             YES)
set(HAVE_LIBZ               YES)
set(HAVE_LIMITS_H           YES)
set(HAVE_LOCALE_H           YES)
set(HAVE_LONGLONG           YES)
set(HAVE_MEMORY_H           YES)
set(HAVE_NETDB_H            YES)
set(HAVE_NETINET_IN_H       YES)
set(HAVE_OPENSSL_CRYPTO_H   YES)
set(HAVE_OPENSSL_ERR_H      YES)
set(HAVE_OPENSSL_PEM_H      YES)
set(HAVE_OPENSSL_PKCS12_H   YES)
set(HAVE_OPENSSL_RSA_H      YES)
set(HAVE_OPENSSL_SSL_H      YES)
set(HAVE_OPENSSL_X509_H     YES)
set(HAVE_PEM_H              YES)
set(HAVE_PERROR             YES)
set(HAVE_PIPE               YES)
set(HAVE_POLL               YES)
set(HAVE_POLL_H             YES)
set(HAVE_POSIX_STRERROR_R   ${NO_WIN32})
set(HAVE_PTHREAD_H          YES)
set(HAVE_RAND_EGD           YES)
set(HAVE_RAND_STATUS        YES)

set(HAVE_RECV               YES)
set(HAVE_RECVFROM           YES)
set(HAVE_SELECT             YES)
set(HAVE_SEND               YES)
set(HAVE_SETJMP_H           YES)
set(HAVE_SETLOCALE          YES)
set(HAVE_SETMODE            YES)
set(HAVE_SETSOCKOPT         YES)
set(HAVE_SIGNAL             YES)
set(HAVE_SIGNAL_H           YES)
set(HAVE_SIG_ATOMIC_T       YES)
set(HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID YES)
set(HAVE_SOCKET             YES)
set(HAVE_SSL_GET_SHUTDOWN   YES)
set(HAVE_SSL_H              YES)
set(HAVE_STDBOOL_H          YES)
set(HAVE_STDINT_H           YES)
set(HAVE_STDIO_H            YES)
set(HAVE_STDLIB_H           YES)
set(HAVE_STRCASECMP         ${NO_WIN32})
set(HAVE_STRCASESTR         YES)
set(HAVE_STRDUP             YES)
set(HAVE_STRERROR_R         ${NO_WIN32})
set(HAVE_STRICMP_H          ${YES_WIN32})
set(HAVE_STRING_H           YES)
set(HAVE_STRNCASECMP        ${NO_WIN32})
set(HAVE_STRNICMP_H         ${YES_WIN32})
set(HAVE_STRSTR             YES)
set(HAVE_STRTOLL            YES)
set(HAVE_STRUCT_SOCKADDR_STORAGE YES)
set(HAVE_STRUCT_TIMEVAL     YES)

set(HAVE_SYS_IOCTL_H        YES)
set(HAVE_SYS_RESOURCE_H     YES)
set(HAVE_SYS_SELECT_H       YES)
set(HAVE_SYS_SOCKET_H       YES)
set(HAVE_SYS_STAT_H         YES)
set(HAVE_SYS_TIME_H         YES)
set(HAVE_SYS_TYPES_H        YES)
set(HAVE_SYS_UIO_H          YES)
set(HAVE_TIME_H             YES)
set(HAVE_UNISTD_H           YES)
set(HAVE_UTIME              YES)
set(HAVE_SYS_UTIME_H        YES)

set(HAVE_WINDOWS_H          NO)
set(HAVE_WINSOCK2_H         ${YES_WIN32}) # FIXME: Set to `NO`.

set(HAVE_WRITABLE_ARGV      YES)
set(HAVE_WRITEV             YES)
set(HAVE_X509_H             YES)
set(HAVE_ZLIB_H             YES)

if (NOT ANDROID)
check_type_size(size_t  SIZEOF_SIZE_T)
check_type_size("long long"  SIZEOF_LONG_LONG)
check_type_size("long"  SIZEOF_LONG)
check_type_size("short"  SIZEOF_SHORT)
check_type_size("int"  SIZEOF_INT)
check_type_size("__int64"  SIZEOF___INT64)
check_type_size("long double"  SIZEOF_LONG_DOUBLE)
check_type_size("time_t"  SIZEOF_TIME_T)
else ()
set(SIZEOF_SIZE_T 4)
set(SIZEOF_LONG_LONG 8)
set(SIZEOF_LONG 4)
set(SIZEOF_SHORT 2)
set(SIZEOF_INT 4)
set(SIZEOF___INT64 8)
set(SIZEOF_LONG_DOUBLE 16)
set(SIZEOF_TIME_T ${SIZEOF_LONG})
set(SIZEOF_OFF_T ${SIZEOF_LONG})
set(SIZEOF_VOIDP ${SIZEOF_INT})
endif ()

# Different sizeofs, etc.

#    define CURL_SIZEOF_LONG        4
#    define CURL_TYPEOF_CURL_OFF_T  long long
#    define CURL_FORMAT_CURL_OFF_T  "lld"
#    define CURL_FORMAT_CURL_OFF_TU "llu"
#    define CURL_FORMAT_OFF_T       "%lld"
#    define CURL_SIZEOF_CURL_OFF_T  8
#    define CURL_SUFFIX_CURL_OFF_T  LL
#    define CURL_SUFFIX_CURL_OFF_TU ULL

set(CURL_SIZEOF_LONG ${SIZEOF_LONG})

if(SIZEOF_LONG EQUAL 8)
  set(CURL_TYPEOF_CURL_OFF_T long)
  set(CURL_SIZEOF_CURL_OFF_T 8)
  set(CURL_FORMAT_CURL_OFF_T "ld")
  set(CURL_FORMAT_CURL_OFF_TU "lu")
  set(CURL_FORMAT_OFF_T "%ld")
  set(CURL_SUFFIX_CURL_OFF_T L)
  set(CURL_SUFFIX_CURL_OFF_TU UL)
endif(SIZEOF_LONG EQUAL 8)

if(SIZEOF_LONG_LONG EQUAL 8)
  set(CURL_TYPEOF_CURL_OFF_T "long long")
  set(CURL_SIZEOF_CURL_OFF_T 8)
  set(CURL_FORMAT_CURL_OFF_T "lld")
  set(CURL_FORMAT_CURL_OFF_TU "llu")
  set(CURL_FORMAT_OFF_T "%lld")
  set(CURL_SUFFIX_CURL_OFF_T LL)
  set(CURL_SUFFIX_CURL_OFF_TU ULL)
endif(SIZEOF_LONG_LONG EQUAL 8)

if(NOT CURL_TYPEOF_CURL_OFF_T)
  set(CURL_TYPEOF_CURL_OFF_T ${ssize_t})
  set(CURL_SIZEOF_CURL_OFF_T ${SIZEOF_SSIZE_T})
  # TODO: need adjustment here.
  set(CURL_FORMAT_CURL_OFF_T "ld")
  set(CURL_FORMAT_CURL_OFF_TU "lu")
  set(CURL_FORMAT_OFF_T "%ld")
  set(CURL_SUFFIX_CURL_OFF_T L)
  set(CURL_SUFFIX_CURL_OFF_TU LU)
endif(NOT CURL_TYPEOF_CURL_OFF_T)

if(HAVE_SIZEOF_LONG_LONG)
  set(HAVE_LONGLONG 1)
  set(HAVE_LL 1)
endif(HAVE_SIZEOF_LONG_LONG)

find_file(RANDOM_FILE urandom /dev)
mark_as_advanced(RANDOM_FILE)

set(RECV_TYPE_ARG1          "int")
set(RECV_TYPE_ARG2          "void *")
set(RECV_TYPE_ARG3          "size_t")
set(RECV_TYPE_ARG4          "unsigned int")
set(RECV_TYPE_RETV          "ssize_t")

set(SEND_QUAL_ARG2          "const")
set(SEND_TYPE_ARG1          "int")
set(SEND_TYPE_ARG2          "void *")
set(SEND_TYPE_ARG3          "size_t")
set(SEND_TYPE_ARG4          "unsigned int")
set(SEND_TYPE_RETV          "ssize_t")

set(USE_OPENSSL             YES)

include_directories(${CMAKE_SOURCE_DIR}/boringssl/include)

if(WIN32)
  add_definitions(-DALLOW_MSVC6_WITHOUT_PSDK)
  include_directories(${CMAKE_SOURCE_DIR}/zlib)
  set(CURL_LIBS ssl crypto zlib)
endif(WIN32)

# (WebKit) Removed configuration calculations

add_definitions(-DHAVE_CONFIG_H)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

# Ugly (but functional) way to include "Makefile.inc" by transforming it (= regenerate it).
function(TRANSFORM_MAKEFILE_INC INPUT_FILE OUTPUT_FILE)
  file(READ ${INPUT_FILE} MAKEFILE_INC_TEXT)
  string(REPLACE "$(top_srcdir)"   "\${CURL_SOURCE_DIR}" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})
  string(REPLACE "$(top_builddir)" "\${CURL_BINARY_DIR}" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})

  string(REGEX REPLACE "\\\\\n" "?!?" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})
  string(REGEX REPLACE "([a-zA-Z_][a-zA-Z0-9_]*)[\t ]*=[\t ]*([^\n]*)" "SET(\\1 \\2)" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})
  string(REPLACE "?!?" "\n" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})

  string(REGEX REPLACE "\\$\\(([a-zA-Z_][a-zA-Z0-9_]*)\\)" "\${\\1}" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})    # Replace $() with ${}
  string(REGEX REPLACE "@([a-zA-Z_][a-zA-Z0-9_]*)@" "\${\\1}" MAKEFILE_INC_TEXT ${MAKEFILE_INC_TEXT})    # Replace @@ with ${}, even if that may not be read by CMake scripts.
  file(WRITE ${OUTPUT_FILE} ${MAKEFILE_INC_TEXT})

endfunction()

add_subdirectory(lib)
if(BUILD_CURL_EXE)
  add_subdirectory(src)
endif()
if(BUILD_CURL_TESTS)
  add_subdirectory(tests)
endif()

# TODO support GNUTLS, NSS, POLARSSL, AXTLS, CYASSL, WINSSL, DARWINSSL
if(USE_OPENSSL)
  set(SSL_ENABLED 1)
endif()

# Helper to populate a list (_items) with a label when conditions (the remaining
# args) are satisfied
function(_add_if label)
  # TODO need to disable policy CMP0054 (CMake 3.1) to allow this indirection
  if(${ARGN})
    set(_items ${_items} "${label}" PARENT_SCOPE)
  endif()
endfunction()

# Clear list and try to detect available features
set(_items)
_add_if("WinSSL"        SSL_ENABLED AND USE_WINDOWS_SSPI)
_add_if("OpenSSL"       SSL_ENABLED AND USE_OPENSSL)
_add_if("IPv6"          ENABLE_IPV6)
_add_if("unix-sockets"  USE_UNIX_SOCKETS)
_add_if("libz"          HAVE_LIBZ)
_add_if("AsynchDNS"     USE_ARES OR USE_THREADS_POSIX)
_add_if("IDN"           HAVE_LIBIDN)
# TODO SSP1 (WinSSL) check is missing
_add_if("SSPI"          USE_WINDOWS_SSPI)
_add_if("GSS-API"       HAVE_GSSAPI)
# TODO SSP1 missing for SPNEGO
_add_if("SPNEGO"        NOT CURL_DISABLE_CRYPTO_AUTH AND
                        (HAVE_GSSAPI OR USE_WINDOWS_SSPI))
_add_if("Kerberos"      NOT CURL_DISABLE_CRYPTO_AUTH AND
                        (HAVE_GSSAPI OR USE_WINDOWS_SSPI))
# NTLM support requires crypto function adaptions from various SSL libs
# TODO alternative SSL libs tests for SSP1, GNUTLS, NSS, DARWINSSL
if(NOT CURL_DISABLE_CRYPTO_AUTH AND (USE_OPENSSL OR
   USE_WINDOWS_SSPI OR GNUTLS_ENABLED OR NSS_ENABLED OR DARWINSSL_ENABLED))
  _add_if("NTLM"        1)
  # TODO missing option (autoconf: --enable-ntlm-wb)
  _add_if("NTLM_WB"     NOT CURL_DISABLE_HTTP AND NTLM_WB_ENABLED)
endif()
# TODO missing option (--enable-tls-srp), depends on GNUTLS_SRP/OPENSSL_SRP
_add_if("TLS-SRP"       USE_TLS_SRP)
# TODO option --with-nghttp2 tests for nghttp2 lib and nghttp2/nghttp2.h header
_add_if("HTTP2"         USE_NGHTTP2)
string(REPLACE ";" " " SUPPORT_FEATURES "${_items}")
message(STATUS "Enabled features: ${SUPPORT_FEATURES}")

# Clear list and try to detect available protocols
set(_items)
_add_if("HTTP"          NOT CURL_DISABLE_HTTP)
_add_if("HTTPS"         NOT CURL_DISABLE_HTTP AND SSL_ENABLED)
_add_if("FTP"           NOT CURL_DISABLE_FTP)
_add_if("FTPS"          NOT CURL_DISABLE_FTP AND SSL_ENABLED)
_add_if("FILE"          NOT CURL_DISABLE_FILE)
_add_if("TELNET"        NOT CURL_DISABLE_TELNET)
_add_if("LDAP"          NOT CURL_DISABLE_LDAP)
# CURL_DISABLE_LDAP implies CURL_DISABLE_LDAPS
# TODO check HAVE_LDAP_SSL (in autoconf this is enabled with --enable-ldaps)
_add_if("LDAPS"         NOT CURL_DISABLE_LDAPS AND
                        ((USE_OPENLDAP AND SSL_ENABLED) OR
                        (NOT USE_OPENLDAP AND HAVE_LDAP_SSL)))
_add_if("DICT"          NOT CURL_DISABLE_DICT)
_add_if("TFTP"          NOT CURL_DISABLE_TFTP)
_add_if("GOPHER"        NOT CURL_DISABLE_GOPHER)
_add_if("POP3"          NOT CURL_DISABLE_POP3)
_add_if("POP3S"         NOT CURL_DISABLE_POP3 AND SSL_ENABLED)
_add_if("IMAP"          NOT CURL_DISABLE_IMAP)
_add_if("IMAPS"         NOT CURL_DISABLE_IMAP AND SSL_ENABLED)
_add_if("SMTP"          NOT CURL_DISABLE_SMTP)
_add_if("SMTPS"         NOT CURL_DISABLE_SMTP AND SSL_ENABLED)
_add_if("SCP"           USE_LIBSSH2)
_add_if("SFTP"          USE_LIBSSH2)
_add_if("RTSP"          NOT CURL_DISABLE_RTSP)
_add_if("RTMP"          USE_LIBRTMP)
list(SORT _items)
string(REPLACE ";" " " SUPPORT_PROTOCOLS "${_items}")
message(STATUS "Enabled protocols: ${SUPPORT_PROTOCOLS}")

# curl-config needs the following options to be set.
set(CC                      "${CMAKE_C_COMPILER}")
# TODO probably put a -D... options here?
set(CONFIGURE_OPTIONS       "")
# TODO when to set "-DCURL_STATICLIB" for CPPFLAG_CURL_STATICLIB?
set(CPPFLAG_CURL_STATICLIB  "")
# TODO need to set this (see CURL_CHECK_CA_BUNDLE in acinclude.m4)
set(CURL_CA_BUNDLE          "")
set(CURLVERSION             "${CURL_VERSION}")
set(ENABLE_SHARED           "yes")
if(CURL_STATICLIB)
  # Broken: LIBCURL_LIBS below; .a lib is not built
  message(WARNING "Static linking is broken!")
  set(ENABLE_STATIC         "no")
else()
  set(ENABLE_STATIC         "no")
endif()
set(exec_prefix             "\${prefix}")
set(includedir              "\${prefix}/include")
set(LDFLAGS                 "${CMAKE_SHARED_LINKER_FLAGS}")
set(LIBCURL_LIBS            "")
set(libdir                  "${CMAKE_INSTALL_PREFIX}/lib")
# TODO CURL_LIBS also contains absolute paths which don't work with static -l...
foreach(_lib ${CMAKE_C_IMPLICIT_LINK_LIBRARIES} ${CURL_LIBS})
  set(LIBCURL_LIBS          "${LIBCURL_LIBS} -l${_lib}")
endforeach()
# "a" (Linux) or "lib" (Windows)
string(REPLACE "." "" libext "${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(prefix                  "${CMAKE_INSTALL_PREFIX}")
# Set this to "yes" to append all libraries on which -lcurl is dependent
set(REQUIRE_LIB_DEPS        "no")
# SUPPORT_FEATURES
# SUPPORT_PROTOCOLS
set(VERSIONNUM              "${CURL_VERSION_NUM}")

# Finally generate a "curl-config" matching this config
configure_file("${CURL_SOURCE_DIR}/curl-config.in"
               "${CURL_BINARY_DIR}/curl-config" @ONLY)
install(FILES "${CURL_BINARY_DIR}/curl-config"
        DESTINATION bin
        PERMISSIONS
          OWNER_READ OWNER_WRITE OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)

# Finally generate a pkg-config file matching this config
configure_file("${CURL_SOURCE_DIR}/libcurl.pc.in"
               "${CURL_BINARY_DIR}/libcurl.pc" @ONLY)
install(FILES "${CURL_BINARY_DIR}/libcurl.pc"
        DESTINATION lib/pkgconfig)

# This needs to be run very last so other parts of the scripts can take advantage of this.
if(NOT CURL_CONFIG_HAS_BEEN_RUN_BEFORE)
  set(CURL_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
endif()

# Installation.
# First, install generated curlbuild.h
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/curl/curlbuild.h"
    DESTINATION include/curl )
# Next, install other headers excluding curlbuild.h
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/curl"
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "curlbuild.h" EXCLUDE)


# Workaround for MSVS10 to avoid the Dialog Hell
# FIXME: This could be removed with future version of CMake.
if(MSVC_VERSION EQUAL 1600)
  set(CURL_SLN_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/CURL.sln")
  if(EXISTS "${CURL_SLN_FILENAME}")
    file(APPEND "${CURL_SLN_FILENAME}" "\n# This should be regenerated!\n")
  endif()
endif()
