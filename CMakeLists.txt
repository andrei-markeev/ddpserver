cmake_minimum_required(VERSION 2.6)

project(ddpserver)

include_directories("include/")
link_directories("lib/")

set(LIBRARY_OUTPUT_PATH  "lib/")

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_library(ddpserver STATIC src/ddpserver.cpp)

install (TARGETS ddpserver
	     ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (DIRECTORY include/ DESTINATION include/)
