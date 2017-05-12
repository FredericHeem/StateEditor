set(QSLOG_LIBRARY QsLog CACHE INTERNAL "QsLog library name")
set(QSLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "QsLog include directory")

set(QSLOG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5Core)

set(QSLOG_SRCS QsDebugOutput.cpp QsLog.cpp QsLogDest.cpp)

add_library(${QSLOG_LIBRARY} STATIC ${QSLOG_SRCS})

qt5_use_modules(${QSLOG_LIBRARY} Core)
