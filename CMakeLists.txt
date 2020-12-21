cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)

project(Ubridge)

# include_directories(
#   include
# )

add_compile_options(-Wall)

set(LIBS -lrt -lpthread -ldl)

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