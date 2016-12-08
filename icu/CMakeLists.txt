
if (MSVC)
    string(REGEX REPLACE "/GR- " "/GR " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Enable RTTI
endif ()

add_subdirectory(source/common)
add_subdirectory(source/stubdata)
add_subdirectory(source/i18n)