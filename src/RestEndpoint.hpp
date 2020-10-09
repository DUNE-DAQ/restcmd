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

#include <nlohmann/json.hpp>

#include <pistache/http.h>
#include <pistache/http_header.h>
#include <pistache/description.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/mime.h>

#include <thread>
#include <chrono>
#include <future>
#include <memory>
#include <string>

namespace dunedaq {
namespace restcmd {

class RestEndpoint {
public: 
  explicit RestEndpoint(const std::string& /*uri*/, int port,
                        std::function<void(const nlohmann::json&)> functor) noexcept 
    : port_{ static_cast<uint16_t>(port) }
    , address_{ Pistache::Ipv4::any(), port_ }
    , http_endpoint_{ std::make_shared<Pistache::Http::Endpoint>( address_ ) }
    , description_{ "DUNE DAQ cmdlib API", "0.1" }
    , accepted_mime_{ MIME(Application, Json) }
    , command_callback_{ functor }
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
  void handleRouteCommand(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);

  // REST
  Pistache::Port port_;
  Pistache::Address address_;
  std::shared_ptr<Pistache::Http::Endpoint> http_endpoint_;
  Pistache::Rest::Description description_;
  Pistache::Rest::Router router_;
  Pistache::Http::Mime::MediaType accepted_mime_;

  // Function to call with received POST bodies
  std::function<void(const nlohmann::json&)> command_callback_;

  // Background server thread
  std::thread server_thread_;

};

} // namespace restcmd
} // namespace dunedaq

#endif // RESTCMD_SRC_RESTENDPOINT_HPP_ 
