cmake_minimum_required(VERSION 3.25)

project(hands-free-chess-clock LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
  set(WHISPER_CUBLAS 1)
endif()

find_package(ALSA REQUIRED)
include_directories(${ALSA_INCLUDE_DIRS})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(whisper.cpp
  GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
  GIT_TAG v1.5.4
)
FetchContent_MakeAvailable(whisper.cpp)

set(ONNXRUNTIME_ROOTDIR "$ENV{HOME}/onnxruntime-linux-x64-1.17.0")
include_directories("${ONNXRUNTIME_ROOTDIR}/include")
set(onnxruntime_lib ${ONNXRUNTIME_ROOTDIR}/lib/libonnxruntime.so)

add_executable(${PROJECT_NAME})

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

target_sources(${PROJECT_NAME} PUBLIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
  ${ALSA_LIBRARIES}
  ${OpenCV_LIBS}
  whisper
  ${onnxruntime_lib}
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  unit_tests
  src/chess_engine.cpp
  test/unit_tests.cpp
)
target_link_libraries(
  unit_tests
  GTest::gtest_main
  ${ALSA_LIBRARIES}
  ${OpenCV_LIBS}
  whisper
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

add_custom_command(
     TARGET unit_tests
     COMMENT "Run tests"
     POST_BUILD
     COMMAND unit_tests
)
