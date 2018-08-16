cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(EXECUTABLE_OUTPUT_PATH "build")
add_executable(udperf_sender udperf_sender.cpp)
add_executable(udperf_sink udperf_sink.cpp)

find_package(Boost)
IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    find_package (Boost COMPONENTS date_time thread)
    target_link_libraries(udperf_sender Boost::date_time Boost::thread)
    target_link_libraries(udperf_sink Boost::date_time Boost::thread)
endif()

install(TARGETS udperf_sender udperf_sink DESTINATION bin)
