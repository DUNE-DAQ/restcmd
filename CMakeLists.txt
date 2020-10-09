cmake_minimum_required(VERSION 3.12)
project(restcmd VERSION 0.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../daq-buildtools/cmake ${CMAKE_MODULE_PATH})
include(DAQ)

daq_setup_environment()

find_package(cetlib REQUIRED)
find_package(ers REQUIRED)
find_package(cmdlib REQUIRED)
find_package(Pistache REQUIRED)

set(RESTCMD_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers cmdlib::cmdlib ${Pistache_LIBRARIES}/libpistache-0.0.002-git20200802.so)

##############################################################################
daq_point_build_to( src )

add_library(restcmd_restCommandFacility_duneCommandFacility src/RestCommandFacility.cpp src/RestEndpoint.cpp)

target_include_directories(restcmd_restCommandFacility_duneCommandFacility PUBLIC 
  #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
  #$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  ${Pistache_INCLUDE_DIRS}
)

target_link_libraries(restcmd_restCommandFacility_duneCommandFacility ${RESTCMD_DEPENDENCIES})

##############################################################################
daq_point_build_to( test )

add_executable(restcmd_test_rest_app test/test_rest_app.cxx)
target_link_libraries(restcmd_test_rest_app restcmd_restCommandFacility_duneCommandFacility)

##############################################################################
daq_point_build_to( unittest )

daq_install(TARGETS 
  restcmd_restCommandFacility_duneCommandFacility 
  restcmd_test_rest_app
)
