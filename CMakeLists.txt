cmake_minimum_required(VERSION 3.25)

project(PhotoResizeTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(OpenCV REQUIRED)
find_package(Boost 1.87.0 REQUIRED COMPONENTS program_options)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(LINK_LIBRARIES "{LINK_LINK_LIBRARIES} gcov")
endif()


add_executable(PhotoResizeTool
    main.cpp
    CommandLineParser.cpp
    photofilefinder.cpp
    PhotoResizer.cpp
)

target_link_libraries(PhotoResizeTool  ${OpenCV_LIBS} ${Boost_LIBRARIES})
