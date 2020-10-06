/**
 * @file ValidPort.hpp
 *
 * Static function to check if an integer is a valid port number
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_SRC_VALIDPORT_HPP_
#define RESTCMD_SRC_VALIDPORT_HPP_

#include <string>
#include <stdexcept>

namespace dune {
namespace daq {
namespace ccm {

class ValidPort {

public:
  static const int MIN_PORT = 0;
  static const int MAX_PORT = 65535;

  static int portNumber(const int& port) {
    if (!(MIN_PORT <= port && port <= MAX_PORT)) {
      throw std::runtime_error(std::to_string(port));
    } else {
      return port;
    }
  }
  
};

} // namespace ccm
} // namespace daq
} // namespace dune

#endif // RESTCMD_SRC_VALIDPORT_HPP_ 
