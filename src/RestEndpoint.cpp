#include <chrono>
#include <future>

#include "RestEndpoint.hpp"

using namespace dune::daq::ccm;
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
}

void RestEndpoint::createRouting()
{
  using namespace Rest;
  Routes::Post(router_, "/command", Routes::bind(&RestEndpoint::handleCommand, this));
}

inline void extendHeader(Http::Header::Collection& headers) 
{
  headers.add<Http::Header::AccessControlAllowOrigin>("*");
  headers.add<Http::Header::AccessControlAllowMethods>("POST,GET");
  headers.add<Http::Header::ContentType>(MIME(Text, Plain));
}

void RestEndpoint::handleCommand(const Rest::Request& request, Http::ResponseWriter response)
{
  auto addr = request.address();
  auto headers = request.headers();
  auto ansport = headers.getRaw("x-answer-port"); // RS: FIXME
  callback_results_.push( std::move(
    std::async(std::launch::async, command_callback_, 
      std::move(request.body()), addr.host(), std::stoi(ansport.value())
    ))
  );
  auto res = response.send(Http::Code::Ok, "Command received\n");
}
