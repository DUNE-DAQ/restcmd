/**
 * @file RestEndpoint.hpp
 *
 * An HTTP REST Backend implementation based on Pistache
 * Defines a single route, to send commands.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_SRC_RESTENDPOINT_HPP_ 
#define RESTCMD_SRC_RESTENDPOINT_HPP_ 

#include "CallbackTypes.hpp"

#include <tbb/concurrent_queue.h>

#include <pistache/http.h>
#include <pistache/http_header.h>
#include <pistache/description.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <thread>
#include <chrono>
#include <future>
#include <memory>
#include <string>

namespace dune {
namespace daq {
namespace ccm {

class RestEndpoint {
public: 
  explicit RestEndpoint(const std::string& /*uri*/, int port,
                        ResultQueue& resultqueue, 
                        RequestCallback functor,
                        std::launch launchpol) noexcept 
    : port_{ port }, address_{ Pistache::Ipv4::any(), port_ }
    , http_endpoint_{ std::make_shared<Pistache::Http::Endpoint>( address_ ) }
    , description_{"DUNE DAQ CCM CtrlNode API", "0.1"}
    , callback_results_{ resultqueue }
    , command_callback_{ functor }
    , launch_policy_{ launchpol }
  { }

  void init(size_t threads);
  void start();
  void stop();
  void shutdown();

private:
  void createRouting();
  void createDescription();
  void serveTask(); 

  // Routes
  void handleCommand(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);

  // REST
  Pistache::Port port_;
  Pistache::Address address_;
  std::shared_ptr<Pistache::Http::Endpoint> http_endpoint_;
  Pistache::Rest::Description description_;
  Pistache::Rest::Router router_;

  // Callback and results
  ResultQueue& callback_results_;
  RequestCallback command_callback_;
  std::launch launch_policy_;

  std::thread server_thread_;
};

} // namespace ccm
} // namespace daq
} // namespace dune

#endif // RESTCMD_SRC_RESTENDPOINT_HPP_ 
