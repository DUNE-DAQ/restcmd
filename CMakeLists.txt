cmake_minimum_required(VERSION 3.12)
project(restcmd VERSION 0.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../daq-buildtools/cmake ${CMAKE_MODULE_PATH})
include(DAQ)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(Pistache REQUIRED)

##############################################################################
daq_point_build_to( src )

add_library(restcmd_restCommandFacility_duneCommandFacility src/RestCommandFacility.cpp src/RestEndpoint.cpp)
target_include_directories (restcmd_restCommandFacility_duneCommandFacility PUBLIC ${Pistache_INCLUDE_DIRS})
target_link_libraries(restcmd_restCommandFacility_duneCommandFacility appfwk::appfwk)

##############################################################################
daq_point_build_to( test )

##############################################################################
daq_point_build_to( unittest )

daq_install(TARGETS restcmd_restCommandFacility_duneCommandFacility)
