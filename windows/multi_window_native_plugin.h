#ifndef FLUTTER_PLUGIN_MULTI_WINDOW_NATIVE_PLUGIN_H_
#define FLUTTER_PLUGIN_MULTI_WINDOW_NATIVE_PLUGIN_H_

#include <flutter/plugin_registrar_windows.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class MultiWindowNativePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  // Add messenger registration
  static void RegisterMessenger(flutter::BinaryMessenger* messenger);

  // Add callbacks for window mgmt
  static void SetCreateWindowCallback(std::function<void(std::vector<std::string>)> callback);
  static void SetCloseWindowCallback(std::function<void(bool isMainWindow, const std::string& windowId)> callback);
  static void SetWindowIdCallback(std::function<void(const std::string& windowId)> callback);

  // Broadcast API
  static void BroadcastToAll(const std::string& method, const flutter::EncodableValue& args);

 private:
  explicit MultiWindowNativePlugin(flutter::PluginRegistrarWindows* registrar);

  static std::vector<flutter::BinaryMessenger*> messengers_;
  static std::function<void(std::vector<std::string>)> on_create_window_;
  static std::function<void(bool isMainWindow, const std::string& windowId)> on_close_window_;
  static std::function<void(const std::string& windowId)> _set_window_id_;
};

#endif  // FLUTTER_PLUGIN_MULTI_WINDOW_NATIVE_PLUGIN_H_
