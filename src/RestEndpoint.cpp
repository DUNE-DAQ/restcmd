/**
 * @file RestEndpoint.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "RestEndpoint.hpp"

#include <ers/Issue.h>

#include <chrono>
#include <future>
#include <utility>
#include <sstream>

using namespace dunedaq::restcmd;
using namespace Pistache;

void RestEndpoint::init(size_t threads)
{
  auto opts = Http::Endpoint::options()
    .threads(static_cast<int>(threads))
    .maxRequestSize(15728640) // 15MB
    .maxResponseSize(1048576) // 1MB 
    .flags(Pistache::Tcp::Options::ReuseAddr)
    .flags(Pistache::Tcp::Options::ReusePort);

  http_endpoint_->init(opts);
  createRouting();
  http_client_options_ = Http::Client::options().threads(static_cast<int>(threads));
  http_client_->init(http_client_options_);
}

void RestEndpoint::start() 
{
  server_thread_ = std::thread(&RestEndpoint::serveTask, this);
}

void RestEndpoint::serveTask() 
{     
  http_endpoint_->setHandler(router_.handler());
  http_endpoint_->serve();
}

void RestEndpoint::shutdown()
{
  http_endpoint_->shutdown();
  server_thread_.join();
  http_client_->shutdown();
}

void RestEndpoint::createRouting()
{
  using namespace Rest;
  Routes::Post(router_, "/command", Routes::bind(&RestEndpoint::handleRouteCommand, this));
}

inline void extendHeader(Http::Header::Collection& headers) 
{
  headers.add<Http::Header::AccessControlAllowOrigin>("*");
  headers.add<Http::Header::AccessControlAllowMethods>("POST,GET");
  headers.add<Http::Header::ContentType>(MIME(Text, Plain));
}

inline 
std::string 
getClientAddress(const Pistache::Rest::Request &request) {
  const auto xff = request.headers().tryGetRaw("X-Forwarded-For");
  if (!xff.isEmpty()) {
    //TODO: Strip of after first comma (to handle chained proxies).
    return xff.get().value();
  }
  return request.address().host();
}

void RestEndpoint::handleRouteCommand(const Rest::Request& request, Http::ResponseWriter response)
{
  auto addr = request.address();
  auto headers = request.headers();
  auto ct = headers.get<Http::Header::ContentType>(); 
  if ( ct->mime() != accepted_mime_ ) {
    auto res = response.send(Http::Code::Not_Acceptable, "Not a JSON command!\n");
  } else {
    std::ostringstream hdrsstr;
    for (const auto&[hk, rh] : headers.rawList()) {
      hdrsstr << hk << ": " << rh.value() << '\n';
    }
    // std::cout << "From: "<< addr.host() << '\n';
    // std::cout << hdrsstr.str() << '\n';

    auto ansport = headers.getRaw("X-Answer-Port"); // RS: FIXME reply using headers
    cmdmeta_t meta{{"command", request.body()}};
    meta["answer-port"] = ansport.value();
    meta["answer-host"] = addr.host();
    command_callback_(meta); // RS: FIXME parse errors
    auto res = response.send(Http::Code::Accepted, "Command received\n");
  }
}

void RestEndpoint::handleResponseCommand(cmdmeta_t & meta) 
{
  std::ostringstream addrstr;
  addrstr << meta["answer-host"] << ":" << meta["answer-port"];
  meta.erase("answer-host");
  meta.erase("answer-port");
  auto response = http_client_->post(addrstr.str()).body(nlohmann::json(meta).dump()).send();
  response.then(
    [&](Http::Response response) {
      std::cout << "Response code = " << response.code() << std::endl;
      // auto body = response.body();
      // if (!body.empty())
        // std::cout << "Response body = " << body << std::endl;
    },
    [&](std::exception_ptr exc) {
      // handle response failure
      try{
        std::rethrow_exception(exc);
      }
      catch (const std::exception &e) {
        std::cout << "Exception " << e.what() << '\n';
      }
    }
  );
  http_client_responses_.push_back(std::move(response));
}

