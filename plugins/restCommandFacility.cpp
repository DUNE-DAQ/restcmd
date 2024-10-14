/**
 * @file RestCommandFacility.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "restcmd/RestEndpoint.hpp"

#include "cmdlib/CommandFacility.hpp"
#include "cmdlib/Issues.hpp"

#include "confmodel/ConnectivityService.hpp"
#include "iomanager/network/ConfigClient.hpp"
#include "iomanager/network/ConfigClientStructs.hpp"

#include "utilities/Resolver.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <logging/Logging.hpp>
#include <tbb/concurrent_queue.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <limits.h>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>

using namespace dunedaq::cmdlib;
using namespace dunedaq::restcmd;

class restCommandFacility : public CommandFacility
{

public:
  // friend backend for calling functions on CF
  friend class RestEndpoint;

  explicit restCommandFacility(std::string uri,
                               std::string session_name,
                               const dunedaq::confmodel::ConnectivityService* connectivity_service)
    : CommandFacility(uri)
    , m_session_name(session_name)
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

    if (connectivity_service != nullptr) {
      auto connectivity_service_port = std::to_string(connectivity_service->get_service()->get_port());
      m_connectivity_client = std::make_unique<dunedaq::iomanager::ConfigClient>(
        connectivity_service->get_host(),
        connectivity_service_port,
        m_session_name,
        std::chrono::milliseconds(connectivity_service->get_interval_ms()));
    }

    if (port == 0 && connectivity_service == nullptr) {
      throw dunedaq::cmdlib::MalformedUri(
        ERS_HERE, "Can't bind the REST API to port 0 without connectivity service", portstr);
    }

    try { // to setup backend
      command_executor_ = std::bind(&inherited::execute_command, this, std::placeholders::_1, std::placeholders::_2);
      rest_endpoint_ = std::make_unique<dunedaq::restcmd::RestEndpoint>(hostname, port, command_executor_);
      rest_endpoint_->init(1); // 1 thread
      TLOG() << "Endpoint open on: " << epname << " host:" << hostname << " port:" << port;

    } catch (const std::exception& ex) {
      ers::error(dunedaq::cmdlib::CommandFacilityInitialization(ERS_HERE, ex.what()));
    }
  }

  void run(std::atomic<bool>& end_marker)
  {

    char* app_name_c = std::getenv("DUNEDAQ_APPLICATION_NAME");
    if (!app_name_c)
      throw dunedaq::restcmd::EnvVarNotFound(ERS_HERE, "DUNEDAQ_APPLICATION_NAME");
    std::string app_name = std::string(app_name_c);

    // Start endpoint
    try {
      rest_endpoint_->start();

      if (m_connectivity_client) {
        int port = rest_endpoint_->getPort();

        char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);
        auto ips = dunedaq::utilities::get_ips_from_hostname(std::string(hostname));

        if (ips.size() == 0)
          throw dunedaq::cmdlib::CommandFacilityInitialization(ERS_HERE, "Could not resolve hostname to IP address");

        TLOG() << "Registering the control endpoint (" << app_name
               << "_control) on the connectivity service: " << ips[0] << ":" << port;
        dunedaq::iomanager::ConnectionRegistration cr;
        cr.uid = app_name + "_control";
        cr.data_type = "RunControlMessage";
        cr.uri = "rest://" + ips[0] + ":" + std::to_string(port);
        cr.connection_type = dunedaq::iomanager::ConnectionType::kSendRecv;
        m_connectivity_client->publish(cr);
      }
    } catch (const std::exception& ex) {
      ers::fatal(dunedaq::cmdlib::RunLoopTerminated(ERS_HERE, ex.what()));
    }

    // Wait until marked
    while (end_marker) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown
    rest_endpoint_->shutdown();
    if (m_connectivity_client) {
      dunedaq::iomanager::ConnectionId ci{ app_name + "_control", "RunControlMessage", m_session_name };
      m_connectivity_client->retract(ci);
    }
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

  std::string m_session_name;
  std::unique_ptr<dunedaq::iomanager::ConfigClient> m_connectivity_client;
};

extern "C"
{
  std::shared_ptr<dunedaq::cmdlib::CommandFacility> make(std::string uri,
                                                         std::string session_name, const dunedaq::confmodel::ConnectivityService* connectivity_service)
  {

    return std::shared_ptr<dunedaq::cmdlib::CommandFacility>(
      new restCommandFacility(uri, session_name, connectivity_service));
  }
}
