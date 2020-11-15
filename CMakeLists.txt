cmake_minimum_required (VERSION 3.14)

SET(CMAKE_INSTALL_PREFIX "/usr")
set (CMAKE_CXX_COMPILER /usr/bin/clang++)

project (icarus)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXE_LINKER_FLAGS "-rdynamic")
set (CMAKE_SHARED_LINKER_FLAGS "-rdynamic")

file (GLOB SRC_FILES "icarus/*.cpp")

set (CMAKE_CXX_FLAGS "-g -Wall")

add_library(icarus SHARED ${SRC_FILES})
install (TARGETS icarus LIBRARY
    DESTINATION lib
)

file (GLOB HEADERS "icarus/*.hpp")
install (FILES ${HEADERS}
    DESTINATION "include/icarus"
)
