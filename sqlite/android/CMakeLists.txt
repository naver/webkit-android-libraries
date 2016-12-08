set(SQLITE_ANDROID_SOURCES
	OldPhoneNumberUtils.cpp
	PhoneNumberUtils.cpp
	PhonebookIndex.cpp
	PhoneticStringUtils.cpp
	Unicode.cpp
	sqlite3_android.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../dist
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(LIBRARY_DEPS utils)

add_library(sqlite_android STATIC ${SQLITE_ANDROID_SOURCES})

#link the module like any other
target_link_libraries(sqlite_android ${LIBRARY_DEPS})
