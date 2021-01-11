cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

project(Ubridge)

# include_directories(
#   include
# )

#dependencies:
#todo:  using libserial 1.0 version, apt manager has 0.6rc3, need to manually compile it
# may need to run sudo ldconfig after sudo make install


find_package(nng)

add_compile_options(-Wall -ggdb -O0)
# add_compile_options(-Wall -O2)

link_directories("/usr/local/dir")
set(LIBS -lrt -lnng -lpthread -ldl -lserial)

add_executable(ubridge
	src/main.cpp
	src/uSerial.cpp
	src/uDevice.cpp
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