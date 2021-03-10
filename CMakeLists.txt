cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

project(Ubridge)

#dependencies:
#todo:  using libserial 1.0 version, apt manager has 0.6rc3, need to manually compile it
# may need to run sudo ldconfig after sudo make install

find_package(nng)

# add_compile_options(-Wall -ggdb3 -O0) #debug
# add_compile_options(-Wall -O3) #release
add_compile_options(-Wall -O3 -Wno-format-truncation -Wno-psabi) #no-psabi suppress a warning on the JSOn library for GCC 8.x compiler

link_directories("/usr/local/lib")
set(LIBS -lrt -lnng -lpthread -ldl -lserial -lstdc++fs -latomic) #stdc++fs and atomic is needed for GCC8.x 

add_executable(ubridge
	src/main.cpp
	src/uSerial.cpp
	src/uDevice.cpp
	src/uStreamer.cpp
	src/reqRepServer.cpp
	src/ubridge.cpp
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

install(TARGETS ubridge DESTINATION bin COMPONENT binaries)
install(TARGETS consumer DESTINATION bin COMPONENT binaries)
install(FILES ubridgeConfig.json DESTINATION /etc/ubridge COMPONENT config)
install(FILES ubridge.service ubridge-server.service DESTINATION /etc/systemd/system/ COMPONENT service)

#  cd build && cmake -GNinja ..
#  ninja