cmake_minimum_required(VERSION 2.8)

project(Armor)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Armor_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Armor src/main.cpp)

link_directories({OpenCV_LIBRARY_DIRS})

target_link_libraries(Armor ${OpenCV_LIBS})
