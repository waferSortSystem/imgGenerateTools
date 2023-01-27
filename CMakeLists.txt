cmake_minimum_required(VERSION 3.14)

# set the project name
set(PROJECT_NAME imgGenerateTools)

project(${PROJECT_NAME} VERSION 0.1.0)

find_package( OpenCV REQUIRED )

include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

aux_source_directory(. DIR_SRCS)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})