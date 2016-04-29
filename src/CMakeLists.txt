include(jvar)

set(DDP_INCLUDES "../include")

file(GLOB DDP_HEADERS
    "${DDP_INCLUDES}/*.h"
)

include_directories(
	${DDP_INCLUDES}
	${JVAR_INCLUDES}
)

#TODO: Check compiler for modern C++ support
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_library(ddpserver STATIC ddpserver.cpp)

install(TARGETS ddpserver
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install(FILES ${DDP_HEADERS} DESTINATION include)