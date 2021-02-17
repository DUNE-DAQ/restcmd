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

#include <ers/ers.h>
#include <cetlib/BasicPluginFactory.h>
#include <tbb/concurrent_queue.h>

#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

using namespace dunedaq::cmdlib;
using namespace dunedaq::restcmd;

class restCommandFacility : public CommandFacility {

public:
    // friend backend for calling functions on CF
    friend class RestEndpoint;

    explicit restCommandFacility(std::string uri) : CommandFacility(uri) {
      //folly::Uri furi(uri); // Tempting, but I won't depend on folly just for this
      //ERS_INFO("Parsed uri -> scheme:" << furi.scheme() << " host:" << furi.host() << " port:" << furi.port());

      // Parse URI
      auto col = uri.find_last_of(':');
      auto at  = uri.find('@');
      auto sep = uri.find("://");
      if (col == std::string::npos || sep == std::string::npos) { // enforce URI
        throw dunedaq::cmdlib::MalformedUriError(ERS_HERE, "Malformed URI: ", uri);
      }
      std::string scheme = uri.substr(0, sep);
      std::string iname = uri.substr(sep+3);
      if (iname.empty()) {
        throw dunedaq::cmdlib::MalformedUriError(ERS_HERE, "Missing interface name in ", uri);
      }
      std::string portstr = uri.substr(col+1);
      if (portstr.empty() || portstr.find(iname) != std::string::npos ) {
        throw dunedaq::cmdlib::MalformedUriError(ERS_HERE, "Can't bind without port in ", uri);
      }
      std::string epname = uri.substr(sep+3, at-(sep+3));
      std::string hostname = uri.substr(at+1, col-(at+1));

      int port = -1; 
      try { // to parse port
        port = std::stoi(portstr);
        if (!(0 <= port && port <= 65535)) { // valid port
          throw dunedaq::cmdlib::MalformedUriError(ERS_HERE, "Invalid port ", portstr); 
        }
      }
      catch (const std::exception& ex) {
        throw dunedaq::cmdlib::MalformedUriError(ERS_HERE, ex.what(), portstr);
      }

      try { // to setup backend
        command_executor_ = std::bind(&inherited::executeCommand, this, std::placeholders::_1);
        rest_endpoint_= std::make_unique<dunedaq::restcmd::RestEndpoint>(hostname, port, command_executor_);
        rest_endpoint_->init(1); // 1 thread
        ERS_INFO("Endpoint open on: " << epname << " host:" << hostname << " port:" << portstr);
      } 
      catch (const std::exception& ex) {
         ers::error(dunedaq::cmdlib::CommandFacilityError(ERS_HERE, ex.what())); 
      }
    }

    void run(std::atomic<bool>& end_marker) {
      // Start endpoint
      try {
        rest_endpoint_->start();
      } 
      catch (const std::exception& ex) {
        ers::error(dunedaq::cmdlib::CommandFacilityError(ERS_HERE, ex.what()));
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
    void completionCallback(cmdmeta_t& meta) {
      ERS_INFO("Completion result: " << meta.at("result"));
      ERS_INFO("Completion ansport: " << meta.at("answer-port"));
      ERS_INFO("Completion command: " << meta.at("command"));
      rest_endpoint_->handleResponseCommand();
    }

private:
    // Manager, HTTP REST Endpoint and backend resources
    mutable std::unique_ptr<RestEndpoint> rest_endpoint_;

    typedef std::function<void(cmdmeta_t)> RequestCallback;
    RequestCallback command_executor_;

};

extern "C" {
    std::shared_ptr<dunedaq::cmdlib::CommandFacility> make(std::string uri) { 
        return std::shared_ptr<dunedaq::cmdlib::CommandFacility>(new restCommandFacility(uri));
    }
}
