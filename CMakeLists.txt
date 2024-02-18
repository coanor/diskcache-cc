cmake_minimum_required(VERSION 3.10)
project(diskcache-cc VERSION 1.0.0 DESCRIPTION "diskcache in C++ implementation")

include(CTest)

include(CheckIncludeFile)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(3rdparty/spdlog/include)

add_subdirectory(3rdparty/googletest-1.14.0)

set(CMAKE_CXX_STANDARD 17)
add_library(diskcache-cc SHARED
	src/get.cc
	src/open.cc
	src/pos.cc
	src/put.cc
)

set_target_properties(diskcache-cc PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

set(PUB_HEADERS
	"include/disk_cache.hpp"
	"include/err.hpp"
	"include/pos.hpp"
)
set_target_properties(diskcache-cc PROPERTIES PUBLIC_HEADER ${PUB_HEADERS})

include(GNUInstallDirs)
install(TARGETS diskcache-cc
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_file(diskcache-cc.pc.in diskcache-cc.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/diskcache-cc.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

# Testing
# refer to http://google.github.io/googletest/quickstart-cmake.html
enable_testing()
add_executable(T tests/get.cc tests/put.cc tests/open.cc)
target_link_libraries(T GTest::gtest_main diskcache-cc)
include(GoogleTest)
gtest_discover_tests(T)
