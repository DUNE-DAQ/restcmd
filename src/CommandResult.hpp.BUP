/**
 * @file CommandResult.hpp
 *
 * CommandResult is a simple struct to hold the data about the 
 * request's success and to whom to respond with the result.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_SRC_COMMANDRESULT_HPP_
#define RESTCMD_SRC_COMMANDRESULT_HPP_

#include <string>

namespace dune {
namespace daq {
namespace ccm { 

struct CommandResult
{
  CommandResult(const std::string& cliaddr, int cliport, const std::string& result)
    : answer_addr_{cliaddr}, answer_port_(cliport), result_{result}
  {}
  std::string answer_addr_; // caller address
  int answer_port_;    // caller's requested port to answer
  std::string result_;      // result content
};

} // namespace ccm
} // namespace daq
} // namespace dune

#endif // RESTCMD_SRC_COMMANDRESULT_HPP_
