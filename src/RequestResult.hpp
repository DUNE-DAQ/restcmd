#pragma once

#include <string>

namespace dune {
namespace daq {
namespace ccm { 

struct RequestResult
{
  RequestResult(const std::string& cliaddr, uint16_t cliport, const std::string& result)
    : answer_addr_{cliaddr}, answer_port_(cliport), result_{result}
  {}
  std::string answer_addr_; // caller address
  uint16_t answer_port_;    // caller's requested port to answer
  std::string result_;      // result content
};

}
}
}
