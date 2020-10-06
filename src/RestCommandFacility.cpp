/** @file RestCommandFacility.cpp

    @author Roland Sipos - rsipos@cern.ch
 */

#include "appfwk/CommandFacility.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/DAQModuleManager.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <nlohmann/json.hpp>

#include <tbb/concurrent_queue.h>

#include <fstream>
#include <string>
#include <memory>
#include <csignal>
#include <chrono>
#include <functional>

#include "ValidPort.hpp"
#include "CallbackTypes.hpp"
#include "RestEndpoint.hpp"

using namespace dunedaq::appfwk;
using namespace std::literals::chrono_literals;
namespace ccm = dune::daq::ccm;
using object_t = nlohmann::json;

// Global signal carrier
volatile int global_signal;


struct restCommandFacility : public CommandFacility {

    // Manager, HTTP REST Endpoint and backend resources
    mutable DAQModuleManager* manager_ptr_;
    mutable std::unique_ptr<ccm::RestEndpoint> rest_endpoint_;
    ccm::ResultQueue result_queue_;
    ccm::RequestCallback command_callback_;
    std::thread response_handler_;
    std::launch policy_ = std::launch::deferred; 

    // Sighandler
    static void sigHandler(int signal) {
      ERS_INFO("Signal to stop received: " << signal);
      global_signal = signal;
    }

    // Command callback
    ccm::RequestResult commandHandler(const std::string& command, std::string ansaddr, uint16_t port) {
      ccm::RequestResult rr(ansaddr, port, "");
      try {
        manager_ptr_->execute( object_t::parse(command) );
        rr.result_ = "OK";
      } 
      catch (const std::exception& e) {
        rr.result_ = e.what();
        //ers::error(InternalError(ERS_HERE, e.what())); // we don't throw, but store it as result?
      }
      return rr;
    }

    void responseHandler() {
      std::future<ccm::RequestResult> fut;
      while (!global_signal) {
        if (result_queue_.empty()) {
          std::this_thread::sleep_for(1s);
        } 
        else {
          bool success = result_queue_.try_pop(fut);
          if (success) { // fut has content
            fut.wait();
            auto res = fut.get();
            ERS_INFO("Answer to .. " << res.answer_addr_ << " " << res.answer_port_ << " " << res.result_);
            // Client POST should come here...
          }
        } 
      } // while
    }

    // DTOR
    virtual ~restCommandFacility() {
      response_handler_.join();
    }

    // CTOR
    restCommandFacility(std::string uri) : CommandFacility(uri) {
      // Setup sighandler
      std::signal(SIGQUIT, restCommandFacility::sigHandler);       
      std::signal(SIGKILL, restCommandFacility::sigHandler);
      std::signal(SIGABRT, restCommandFacility::sigHandler);

      // Parse URI
      auto col = uri.find_last_of(':');
      auto at  = uri.find('@');
      auto sep = uri.find("://");
      if (col == std::string::npos || sep == std::string::npos) { // enforce URI
        throw UnsupportedUri(ERS_HERE, uri);
      }
      std::string scheme = uri.substr(0, sep);
      std::string iname = uri.substr(sep+3);
      std::string portstr = uri.substr(col+1);
      std::string epname = uri.substr(sep+3, at-(sep+3));
      std::string hostname = uri.substr(at+1, col-(at+1));
      ERS_INFO("Endpoint: " << epname << " host:" << hostname << " port:" << portstr);
      ERS_INFO("  -> open with protocol:" << scheme);
   
      // Bind callback function
      command_callback_ = std::bind(&restCommandFacility::commandHandler, this, 
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
      );

      // Setup response handler thread
      response_handler_ = std::thread(&restCommandFacility::responseHandler, this);

      // Setup endpoint 
      try {
        const uint16_t port = ccm::ValidPort::portNumber(std::stoi(portstr));
        rest_endpoint_= std::make_unique<ccm::RestEndpoint>(hostname, port, result_queue_, command_callback_, policy_);
        rest_endpoint_->init(1); // 1 thread
      }
      catch (const std::exception& e) {
        ers::error(UnsupportedUri(ERS_HERE, e.what()));
      }

    }

    void run(DAQModuleManager& manager) const {
      // setup manager
      manager_ptr_ = &manager;

      // Start endpoint
      try {
        rest_endpoint_->start();
      } 
      catch (const std::exception& e) {
        ers::error(UnsupportedUri(ERS_HERE, e.what()));
      }

      // Wait until
      while ( !global_signal ) {
        std::this_thread::sleep_for(1s);
      }

      // Shutdown
      rest_endpoint_->shutdown();
      manager_ptr_ = nullptr;
    }
};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::CommandFacility> make(std::string uri) { 
        return std::shared_ptr<dunedaq::appfwk::CommandFacility>(new restCommandFacility(uri));
    }
}
