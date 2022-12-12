/**
 * @file RestCommandFacility.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "RestEndpoint.hpp"

#include "cmdlib/CommandFacility.hpp"
#include "cmdlib/Issues.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <logging/Logging.hpp>
#include <tbb/concurrent_queue.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

using namespace dunedaq::cmdlib;
using namespace dunedaq::restcmd;

class restCommandFacility : public CommandFacility
{

public:
  // friend backend for calling functions on CF
  friend class RestEndpoint;

  explicit restCommandFacility(std::string uri)
    : CommandFacility(uri)
  {

    // Parse URI
    auto col = uri.find_last_of(':');
    auto at = uri.find('@');
    auto sep = uri.find("://");
    if (col == std::string::npos || sep == std::string::npos) { // enforce URI
      throw dunedaq::cmdlib::MalformedUri(ERS_HERE, "Malformed URI: ", uri);
    }
    std::string scheme = uri.substr(0, sep);
    std::string iname = uri.substr(sep + 3);
    if (iname.empty()) {
      throw dunedaq::cmdlib::MalformedUri(ERS_HERE, "Missing interface name in ", uri);
    }
    std::string portstr = uri.substr(col + 1);
    if (portstr.empty() || portstr.find(iname) != std::string::npos) {
      throw dunedaq::cmdlib::MalformedUri(ERS_HERE, "Can't bind without port in ", uri);
    }
    std::string epname = uri.substr(sep + 3, at - (sep + 3));
    std::string hostname = uri.substr(at + 1, col - (at + 1));

    int port = -1;
    try { // to parse port
      port = std::stoi(portstr);
      if (!(0 <= port && port <= 65535)) { // valid port
        throw dunedaq::cmdlib::MalformedUri(ERS_HERE, "Invalid port ", portstr);
      }
    } catch (const std::exception& ex) {
      throw dunedaq::cmdlib::MalformedUri(ERS_HERE, ex.what(), portstr);
    }

    try { // to setup backend
      command_executor_ = std::bind(&inherited::execute_command, this, std::placeholders::_1, std::placeholders::_2);
      rest_endpoint_ = std::make_unique<dunedaq::restcmd::RestEndpoint>(hostname, port, command_executor_);
      rest_endpoint_->init(1); // 1 thread
      TLOG() << "Endpoint open on: " << epname << " host:" << hostname << " port:" << portstr;
    } catch (const std::exception& ex) {
      ers::error(dunedaq::cmdlib::CommandFacilityInitialization(ERS_HERE, ex.what()));
    }
  }

  void run(std::atomic<bool>& end_marker)
  {
    // Start endpoint
    try {
      rest_endpoint_->start();
    } catch (const std::exception& ex) {
      ers::fatal(dunedaq::cmdlib::RunLoopTerminated(ERS_HERE, ex.what()));
    }

    // Wait until marked
    while (end_marker) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown
    rest_endpoint_->shutdown();
  }

protected:
  typedef CommandFacility inherited;

  // Implementation of completionHandler interface
  void completion_callback(const cmdobj_t& cmd, cmd::CommandReply& meta)
  {
    rest_endpoint_->handleResponseCommand(cmd, meta);
  }

private:
  // Manager, HTTP REST Endpoint and backend resources
  mutable std::unique_ptr<RestEndpoint> rest_endpoint_;

  typedef std::function<void(const cmdobj_t&, cmd::CommandReply)> RequestCallback;
  RequestCallback command_executor_;
};

extern "C"
{
  std::shared_ptr<dunedaq::cmdlib::CommandFacility> make(std::string uri)
  {
    return std::shared_ptr<dunedaq::cmdlib::CommandFacility>(new restCommandFacility(uri));
  }
}
