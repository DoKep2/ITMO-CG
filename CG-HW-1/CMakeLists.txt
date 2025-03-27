cmake_minimum_required(VERSION 3.10)

project(DirectXExample)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -D_DEBUG -D_CONSOLE -D_UNICODE -DUNICODE -fno-strict-aliasing")

set(DIRECTX_SDK_PATH "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")
set(DIRECTX_LIB_PATH "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64")

include_directories(${DIRECTX_SDK_PATH}/um ${DIRECTX_SDK_PATH}/shared)

link_directories(${DIRECTX_LIB_PATH})
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/*")

foreach(dir ${INCLUDE_DIRS})
    get_filename_component(dir_path ${dir} PATH)
    include_directories(${dir_path})
endforeach()

include_directories(${CMAKE_SOURCE_DIR}/include/components)

add_executable(DirectXExample ${SOURCES}
        MyGame.cpp
        CG-HW-1.cpp)

# Link libraries
target_link_libraries(DirectXExample d3d11 d2d1 d3dcompiler)
