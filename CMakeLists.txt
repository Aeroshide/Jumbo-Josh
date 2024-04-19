# CMakeList.txt : CMake project for CMakeProject2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("CMakeProject2")
# Set the toolchain
include("F:/.vcpkg/scripts/buildsystems/vcpkg.cmake")


# Add source to this project's executable.
add_executable (CMakeProject2 "src/Main.cpp"  "src/Song.h")

find_package(dpp CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE dpp::dpp)

find_package(Ogg CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE Ogg::ogg)

find_package(OpusFile CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE OpusFile::opusfile)

find_package(mpg123 CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE MPG123::libmpg123 MPG123::libout123 MPG123::libsyn123)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(CMakeProject2 PRIVATE fmt::fmt)


# TODO: Add tests and install targets if needed.
