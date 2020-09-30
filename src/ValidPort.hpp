#pragma once

namespace dune {
namespace daq {
namespace ccm {

class ValidPort {

public:
  static const uint16_t MIN_PORT = 0;
  static const uint16_t MAX_PORT = 65535;

  static uint16_t portNumber(const int& port) {
    if (!(MIN_PORT <= port && port <= MAX_PORT)) {
      throw std::runtime_error(std::to_string(port));
    } 
    else {
      return port;
    }
  }
  
};

}
}
}
