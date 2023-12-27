cmake_minimum_required(VERSION 3.28)

project(hands-free-chess-clock LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_SCAN_FOR_MODULES True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PUBLIC
    src/main.cpp
)
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES FILES
    src/control_loop.cpp
)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
