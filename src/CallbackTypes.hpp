/**
 * @file CallbackTypes.hpp
 *
 * Type definitions for callbacks and request results
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_SRC_CALLBACKTYPES_HPP_
#define RESTCMD_SRC_CALLBACKTYPES_HPP_

#include "CommandResult.hpp"

#include <tbb/concurrent_queue.h>

#include <future>
#include <string>

namespace dune {
namespace daq {
namespace ccm { 

// Type for concurrent queue that holds futures of async calls that returns CommandResults
typedef tbb::concurrent_queue<std::future<CommandResult>> ResultQueue;

// Request Callback function signature
typedef std::function<CommandResult(const std::string&, std::string, int)> RequestCallback;

} // namespace ccm
} // namespace daq
} // namespace dune

#endif  // RESTCMD_SRC_CALLBACKTYPES_HPP_
