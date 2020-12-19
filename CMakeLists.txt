cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

project(ubridge)

# add_library(chucknorris
#     include/ChuckNorris.hpp
#     src/ChuckNorris.cpp
# )


# target_include_directories(
#   chucknorris
#   PUBLIC
#     "include"
# )

add_executable(ubridge
  src/main.cpp
)

# target_link_libraries(cpp_demo chucknorris)
