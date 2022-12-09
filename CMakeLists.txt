cmake_minimum_required(VERSION 3.10)
project(zbuffer)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Add the include directories
include_directories(
    /usr/include/GL
)

# Add the library directories
link_directories(
    /usr/lib/x86_64-linux-gnu
)
# Find the Assimp library
find_package(Assimp REQUIRED)
# Find the OpenGL library
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

# Add the source files for your project
set(SOURCES scanline.cpp)

# Create the executable for your project
add_executable(scanline ${SOURCES})

# Link the Assimp library to your project
target_link_libraries(scanline PRIVATE OpenGL::GL)
target_link_libraries(scanline PRIVATE ${OpenCV_LIBS})
# target_link_libraries(scanline PRIVATE ${Assimp_LIBS})
target_link_libraries(scanline PRIVATE assimp)