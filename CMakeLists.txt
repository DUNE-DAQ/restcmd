cmake_minimum_required(VERSION 3.12)
project(restcmd VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(cetlib REQUIRED)
find_package(ers REQUIRED)
find_package(cmdlib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Pistache REQUIRED)

set(RESTCMD_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers cmdlib::cmdlib nlohmann_json::nlohmann_json pistache_shared)

##############################################################################
# Main library

daq_add_library(RestEndpoint.cpp LINK_LIBRARIES ${RESTCMD_DEPENDENCIES})

##############################################################################
# Plugins

daq_add_plugin(restCommandFacility duneCommandFacility LINK_LIBRARIES restcmd)

##############################################################################
# Integration tests

daq_add_application(restcmd_test_rest_app test_rest_app.cxx TEST LINK_LIBRARIES restcmd)

##############################################################################
# No unit tests written

##############################################################################

daq_install()
