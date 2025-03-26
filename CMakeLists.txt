cmake_minimum_required(VERSION 3.22)

project(VirtualArray)
set(CMAKE_CXX_STANDARD 23)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(TARGET_NAME VirtualArray) 

#file(GLOB VIRTUALARRAY_SOURCES CONFIGURE_DEPENDS 
#	src/*.*
#)

list(APPEND VIRTUALARRAY_SOURCES
	main.cpp
)

add_executable(${TARGET_NAME} ${VIRTUALARRAY_SOURCES})

# set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VirtualArray)