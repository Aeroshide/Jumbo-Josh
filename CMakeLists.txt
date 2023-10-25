# CMakeList.txt : CMake project for CMakeProject2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("CMakeProject2")
# Set the toolchain
include("D:/.vcpkg/scripts/buildsystems/vcpkg.cmake")


# Add source to this project's executable.
add_executable (CMakeProject2 "CMakeProject2.cpp" "CMakeProject2.h")

find_package(dpp CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE dpp::dpp)


# TODO: Add tests and install targets if needed.
