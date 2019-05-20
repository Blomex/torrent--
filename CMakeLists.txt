cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

ADD_EXECUTABLE( netstore-client main.cpp shared_structs.h err.h err.cpp)
ADD_EXECUTABLE( netstore-server netstore-server.cpp shared_structs.h err.h err.cpp)
TARGET_LINK_LIBRARIES( netstore-client LINK_PUBLIC ${Boost_LIBRARIES} )
TARGET_LINK_LIBRARIES( netstore-server LINK_PUBLIC ${Boost_LIBRARIES} )