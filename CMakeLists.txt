project(StateEditor)
cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.0.0/5.0.0/msvc2010")
add_subdirectory(src)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

# Where to find all FindXXX.cmake
#set(CMAKE_MODULE_PATH $ENV{STATEBUILDERCPP_HOME}/share/cmake)
