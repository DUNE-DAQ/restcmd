/** @file HttpCommandFacility.cpp

    Fixme: where should I start...

    Fixme: Style guide: CCM deps should follow appfwk style

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

#include "CtrlNode.hpp"
#include "CCMValid.hpp"
// should be:
// #include "ccm/CtrlNode.hpp"

using namespace dunedaq::appfwk;
using namespace std::literals::chrono_literals;
using object_t = nlohmann::json;

volatile int global_signal;

struct restCommandFacility : public CommandFacility {

    dune::daq::ccm::CtrlNode node_;
    std::future<std::string> command_future_;
    std::vector<std::future<std::string>> command_futures_;
    std::string command_string_;
    std::function<void(const std::string& arg)> command_callback_;

    std::mutex run_lock_;
    std::condition_variable run_cv_;
    bool ready = false;
    bool should_stop = false;

    static void sigHandler(int signal) {
      ERS_INFO("Signal to stop received: " << signal);
      //ERS_INFO("Teardown CtrlNode...");
      //node_.teardown();
      global_signal = signal;
      exit(1);
    }

    void cmdHandler(const std::string& command) {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      manager_.execute( object_t::parse(command) );
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      std::cout << "Execute time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

    virtual ~restCommandFacility() {
        // assure these die first.
        node_.teardown();
    }

    restCommandFacility(std::string uri, DAQModuleManager& manager) : CommandFacility(uri, manager) {
        std::signal(SIGQUIT, restCommandFacility::sigHandler);       
        std::signal(SIGKILL, restCommandFacility::sigHandler);
        std::signal(SIGABRT, restCommandFacility::sigHandler);

        command_callback_ = std::bind(&restCommandFacility::cmdHandler, this, std::placeholders::_1);

        auto col = uri.find_last_of(':');
        auto at  = uri.find('@');
        auto sep = uri.find("://");
        if (col == std::string::npos) { // enforce port
            throw UnsupportedUri(ERS_HERE, uri);
        }

        std::string scheme = "";
        std::string iname = uri;

        if (sep == std::string::npos) { // simple path
            scheme = "file";
            iname = uri;
        }
        else {                  // with scheme
            scheme = uri.substr(0, sep);
            iname = uri.substr(sep+3);
        }

        std::string portstr = uri.substr(col+1);
        std::string nodename = uri.substr(sep+3, at-(sep+3));
        std::string hostname = uri.substr(at+1, col-(at+1));

        ERS_INFO("Node: " << nodename << " host:" << hostname << " port:" << portstr);
        ERS_INFO("open: scheme:" << scheme);
     
        try {
          const uint16_t port = dune::daq::ccm::Valid::portNumber(std::stoi(portstr));
          node_.addControlledObj(nodename, hostname, port, command_future_, command_callback_);
        } catch (const std::exception& e) {
          ers::error(UnsupportedUri(ERS_HERE, e.what()));
        }

    } 

    void run(DAQModuleManager& /*manager*/) const {
        while ( !global_signal ) {
            std::this_thread::sleep_for(1s);
        }

    }

};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::CommandFacility> make(std::string uri, DAQModuleManager& manager) { 
        return std::shared_ptr<dunedaq::appfwk::CommandFacility>(new restCommandFacility(uri, manager));
    }
}
