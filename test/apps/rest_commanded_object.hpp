/**
 * @file rest_commanded_object_.hpp REST commanded object
 * implementation that counts the number of executed commands
 * and prints out statistics about it.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef RESTCMD_TEST_REST_COMMANDED_OBJECT_HPP_
#define RESTCMD_TEST_REST_COMMANDED_OBJECT_HPP_

#include "cmdlib/CommandedObject.hpp"

#include <logging/Logging.hpp>

#include <stdexcept>
#include <string>

struct RestCommandedObject : public dunedaq::cmdlib::CommandedObject
{
  int counter_ = 0;
  std::atomic<bool>& runmarker_;
  std::thread stats_;

  void execute(const dunedaq::cmdlib::cmdobj_t& /*command*/) { ++counter_; }

  explicit RestCommandedObject(std::atomic<bool>& rm)
    : runmarker_(rm)
  {
    stats_ = std::thread([&]() {
      while (runmarker_) {
        TLOG() << "Total number of commands received: " << counter_;
        std::this_thread::sleep_for(std::chrono::seconds(5));
      }
    });
  }

  ~RestCommandedObject()
  {
    if (stats_.joinable()) {
      stats_.join();
    }
  }

  RestCommandedObject(const RestCommandedObject&) = delete; ///< RestCommandedObject is not copy-constructible
  RestCommandedObject& operator=(const RestCommandedObject&) = delete; ///< RestCommandedObject is not copy-assignable
  RestCommandedObject(RestCommandedObject&&) = delete;            ///< RestCommandedObject is not move-constructible
  RestCommandedObject& operator=(RestCommandedObject&&) = delete; ///< RestCommandedObject is not move-assignable
};

#endif // RESTCMD_TEST_REST_COMMANDED_OBJECT_HPP_
