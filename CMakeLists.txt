cmake_minimum_required(VERSION 3.12)
project(ccm-ctrl VERSION 0.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../daq-buildtools/cmake ${CMAKE_MODULE_PATH})
include(DAQ)

daq_setup_environment()

find_package(appfwk REQUIRED)
#find_package(ccm REQUIRED)

##############################################################################
daq_point_build_to( src )

message("DUNE_DAQ_CCM_INC -> " $ENV{DUNE_DAQ_CCM_INC})
message("DUNE_DAQ_CCM LIB -> " $ENV{DUNE_DAQ_CCM_LIB})

string(REPLACE ":" ";" LIBRARY_DIRS $ENV{DUNE_DAQ_CCM_LIB})
find_library(ccmControl_LIBRARY libccmControl.so PATHS ${LIBRARY_DIRS})
add_library(ccm-ctrl_restCommandFacility_duneCommandFacility src/RestCommandFacility.cpp)
target_include_directories (ccm-ctrl_restCommandFacility_duneCommandFacility PUBLIC $ENV{DUNE_DAQ_CCM_INC} $ENV{PISTACHE_INC_DIR})
target_link_libraries(ccm-ctrl_restCommandFacility_duneCommandFacility appfwk::appfwk 
  $ENV{DUNE_DAQ_CCM_LIB}/libccmControl.so 
)

##############################################################################
daq_point_build_to( test )

#file(COPY test/driver.json DESTINATION test)

##############################################################################
daq_point_build_to( unittest )

daq_install(TARGETS ccm-ctrl_restCommandFacility_duneCommandFacility)

