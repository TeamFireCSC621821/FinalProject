cmake_minimum_required(VERSION 3.3)
project(FinalProjectBiomed)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


# Find ITK.
set(ITK_DIR /Users/mac/ITK)
find_package(ITK REQUIRED )
include(${ITK_USE_FILE})

# Find VTK
set(VTK_DIR /Users/mac/VTK)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})



set(SOURCE_FILES main.cpp)
add_executable(FinalProjectBiomed ${SOURCE_FILES})

target_link_libraries(FinalProjectBiomed ${ITK_LIBRARIES} ${VTK_LIBRARIES})