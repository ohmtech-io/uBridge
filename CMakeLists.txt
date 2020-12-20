cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)

project(Ubridge)

# add_library(chucknorris
#     include/ChuckNorris.hpp
#     src/ChuckNorris.cpp
# )

# include_directories(
#   include
# )
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
# add_compile_options(-pthread -lrt -Wall)

set(LIBS -lrt -lpthread)

add_executable(ubridge
	src/main.cpp
)

# target_compile_options(ubridge PUBLIC -Wall)
target_include_directories(ubridge PUBLIC "include")
# target_link_libraries(ubridge -lrt -lpthread)
target_link_libraries(ubridge ${LIBS})


add_executable(consumer
	src/consumer.cpp
)

# target_compile_options(consumer PUBLIC -Wall)
target_include_directories(consumer PUBLIC "include")
target_link_libraries(consumer ${LIBS})


#  cd build && cmake -GNinja ..