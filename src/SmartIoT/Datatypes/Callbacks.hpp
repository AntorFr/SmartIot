#pragma once

#include <functional>
#include "../../SmartIotEvent.hpp"
#include "../../SmartIotRange.hpp"

class SmartIotNode;

namespace SmartIotInternals {
  typedef std::function<void()> OperationFunction;

  typedef std::function<bool(const SmartIotNode& node, const SmartIotRange& range, const String& property, const String& value)> GlobalInputHandler;
  typedef std::function<bool(const SmartIotRange& range, const String& property, const String& value)> NodeInputHandler;
  typedef std::function<bool(const SmartIotRange& range, const String& value)> PropertyInputHandler;

  typedef std::function<void(const SmartIotEvent& event)> EventHandler;

  typedef std::function<bool(const String& level, const String& value)> BroadcastHandler;
}  // namespace SmartIotInternals
