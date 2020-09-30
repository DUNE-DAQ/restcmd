#pragma once

#include <future>
#include <string>

#include <tbb/concurrent_queue.h>

#include "RequestResult.hpp"

namespace dune {
namespace daq {
namespace ccm { 

// Type for concurrent queue that holds futures of async calls that returns RequestResults
typedef tbb::concurrent_queue<std::future<RequestResult>> ResultQueue;

// Request Callback function signature
typedef std::function<RequestResult(const std::string&, std::string, uint16_t)> RequestCallback;

}
}
}
