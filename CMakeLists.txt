cmake_minimum_required(VERSION 3.3)
project(FinalProjectBiomed)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#include FLTK
#INCLUDE_DIRECTORIES(/Users/mac/fltk-1.3.x-r11217)

# Find ITK.
set(FLTK_DIR /Users/mac/fltk-1.3.x-r11217)
find_package(FLTK REQUIRED )
#include(${FLTK_USE_FILE})


# Find ITK.
set(ITK_DIR /Users/mac/ITK)
find_package(ITK REQUIRED )
include(${ITK_USE_FILE})

# Find VTK
set(VTK_DIR /Users/mac/VTK)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

FIND_PACKAGE(OpenGL REQUIRED)


set(SOURCE_FILES main.cpp UI.cpp Fl_Justify_Input.cpp SliderInput.cpp SliderFloatInput.cpp)
add_executable(FinalProjectBiomed ${SOURCE_FILES})

target_link_libraries(FinalProjectBiomed ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${FLTK_LIBRARIES} ${OPENGL_LIBRARIES} )