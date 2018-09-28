cmake_minimum_required(VERSION 3.10)
project(ArmCamera C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

include_directories("C:\\Program Files\\ImageMagick-7.0.8-Q16\\include")
LINK_DIRECTORIES("C:\\Program Files\\ImageMagick-7.0.8-Q16\\lib")

add_executable(ArmCamera ArmCamera.c ${SOURCES})
TARGET_LINK_LIBRARIES(ArmCamera CORE_RL_MagickCore_ CORE_RL_MagickWand_)