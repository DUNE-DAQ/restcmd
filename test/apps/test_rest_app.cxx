/**
 * @file test_rest_app.cxx Test application for using the
 * restCommandFacility with a RestCommandedObject.
 * Showcases error handling of bad URIs and proper cleanup
 * of resources via the killswitch.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "cmdlib/CommandFacility.hpp"
#include "cmdlib/CommandedObject.hpp"
#include "rest_commanded_object.hpp"

#include <logging/Logging.hpp>

#include <string>
#include <chrono>
#include <memory>

using namespace dunedaq::cmdlib;

// Expects the created CommandFacilities to have problems.
std::shared_ptr<CommandFacility>
createFacility(const std::string& uri) 
{
  try {
    return makeCommandFacility(uri);
  }
  catch (const std::exception& ex) {
    TLOG() << "Something is wrong -> " << ex.what();
  }
  return nullptr; 
}

int
main(int /*argc*/, char** /*argv[]*/)
{
  // Run marker
  std::atomic<bool> marker{true};

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    TLOG() << "Application will terminate in 20s...";
    std::this_thread::sleep_for(std::chrono::seconds(20));
    marker.store(false);
  });

  // Commanded object
  RestCommandedObject obj(marker);

  // Exercise bad URIs.
  auto fac = createFacility(std::string("rest://"));
  fac = createFacility(std::string("rest://localhost"));
  fac = createFacility(std::string("rest://localhost:"));
  fac = createFacility(std::string("rest://localhost:-1"));
  fac = createFacility(std::string("rest://localhost:9999999999"));

  // Create good facility
  fac = createFacility(std::string("rest://localhost:12345"));

  // Add commanded object to facility
  fac->set_commanded(obj, "pippo");

  // Run until marked
  fac->run(marker);

  // Join local threads
  if (killswitch.joinable()) {
    killswitch.join();
  }

  // Exit
  TLOG() << "Exiting.";
  return 0;
}
