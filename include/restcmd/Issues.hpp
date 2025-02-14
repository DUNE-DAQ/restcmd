/**
 * @file Issues.hpp restcmd specific ERS Issues
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_INCLUDE_RESTCMD_ISSUES_HPP_
#define RESTCMD_INCLUDE_RESTCMD_ISSUES_HPP_


#include "ers/Issue.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.
#include <string>

namespace dunedaq {

/**
 * @brief restcmd specific issues
 * */

  ERS_DECLARE_ISSUE(restcmd, EnvVarNotFound,
                    "The environment variable wasn't set " << env_var,
                    ((std::string)env_var))

}

#endif // RESTCMD_INCLUDE_RESTCMD_ISSUES_HPP_
