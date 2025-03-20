cmake_minimum_required(VERSION 3.10)

project(DirectXExample)

add_definitions(-D_MBCS)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -D__STRICT_ANSI__")

set(DIRECTX_SDK_PATH "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0")
set(DIRECTX_LIB_PATH "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64")

include_directories(${DIRECTX_SDK_PATH}/um ${DIRECTX_SDK_PATH}/shared)
link_directories(${DIRECTX_LIB_PATH})

file(GLOB SOURCES "./*.cpp")
add_executable(DirectXExample ${SOURCES})

target_link_libraries(DirectXExample d3d11 d3dcompiler dxgi d2d1)
