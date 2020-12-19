cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)

project(Ubridge)

# SET(GCC_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
# SET(GCC_LINK_FLAGS	"-lrt -lpthread")

# add_definitions(${GCC_LINK_FLAGS})


# add_library(chucknorris
#     include/ChuckNorris.hpp
#     src/ChuckNorris.cpp
# )



# include_directories(
#   include
# )
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
# add_compile_options(-pthread -lrt -Wall)


add_executable(ubridge
	src/main.cpp
)

target_compile_options(ubridge PUBLIC)

target_include_directories(ubridge PUBLIC "include")

target_link_libraries(ubridge -lrt -lpthread)


#  cd build && cmake -GNinja ..