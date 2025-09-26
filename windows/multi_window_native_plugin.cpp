// #include "multi_window_native_plugin.h"

// // Windows/Flutter imports
// #include <windows.h>
// #include <VersionHelpers.h>
// #include <shellscalingapi.h>
// #include <flutter/dart_project.h>
// #include <flutter/flutter_view_controller.h>
// #include <flutter/method_channel.h>
// #include <flutter/plugin_registrar_windows.h>
// #include <flutter/standard_method_codec.h>

// #include <memory>
// #include <sstream>
// #include <vector>
// #include <string>
// #include <iostream>
// #include <algorithm>

// #include "plugin_flutterwindow.h"
// #include "plugin_win32window.h"
// #include "utils.h"

// namespace multi_window_native {

// // === Multi-window global state ===
// static std::vector<flutter::BinaryMessenger*> messengers;

// struct WindowContext {
//     std::unique_ptr<flutter::FlutterViewController> controller;
//     std::unique_ptr<PluginWin32Window> window;
// };
// static std::vector<WindowContext> windowContexts;
// static flutter::FlutterViewController* mainController = nullptr;

// // Helper: Find controller for Win32Window
// static flutter::FlutterViewController* GetControllerFromWindow(PluginWin32Window* window) {
//     for (auto& ctx : windowContexts) {
//         if (ctx.window.get() == window) {
//             return ctx.controller.get();
//         }
//     }
//     return nullptr;
// }

// // Helper: Close windows (main or secondary)
// static void CloseWindow(flutter::FlutterViewController* controller) {
//     if (!controller) return;
//     if (controller == mainController) {
//         for (auto& ctx : windowContexts) {
//             if (ctx.controller.get() != mainController) {
//                 HWND hwnd = ctx.window->GetHandle();
//                 if (hwnd) {
//                     ::PostMessage(hwnd, WM_CLOSE, 0, 0);
//                 }
//             }
//         }
//         windowContexts.clear();
//         messengers.clear();
//         PostQuitMessage(0);
//     } else {
//         std::cout << "Window is closing..." << std::endl;
//         auto engine = controller->engine();
//         auto messengerPtr = engine->messenger();
//         delete engine;
//         engine = nullptr;
//         messengers.erase(
//             std::remove(messengers.begin(), messengers.end(), messengerPtr),
//             messengers.end()
//         );
//         for (auto& ctx : windowContexts) {
//             if (ctx.controller.get() == controller) {
//                 HWND hwnd = GetAncestor(ctx.controller.get()->view()->GetNativeWindow(), GA_ROOT);
//                 if (hwnd) {
//                     DestroyWindow(hwnd);
//                 }
//                 break;
//             }
//         }
//         windowContexts.erase(std::remove_if(
//             windowContexts.begin(), windowContexts.end(),
//             [controller](const WindowContext& ctx) {
//                 return ctx.controller.get() == controller;
//             }), windowContexts.end());
//     }
// }

// // Helper: Broadcast methods to all messengers/windows
// static void BroadcastMethodToAllWindows(const std::string& method, const flutter::EncodableValue* args) {
//     for (auto* messenger : messengers) {
//         auto channel = std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
//             messenger, "com.coditas.multi_window_native/pluginChannel",
//             &flutter::StandardMethodCodec::GetInstance());
//         std::unique_ptr<flutter::EncodableValue> clonedArgs = args
//             ? std::make_unique<flutter::EncodableValue>(*args)
//             : nullptr;
//         channel->InvokeMethod(method, std::move(clonedArgs));
//     }
// }

// // Helper: Register messenger/channel for a window
// static void RegisterMessenger(flutter::FlutterViewController* controller) {
//     auto* messenger = controller->engine()->messenger();
//     for (auto* m : messengers)
//         if (m == messenger) return;
//     messengers.push_back(messenger);

//     auto channel = std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
//         messenger,"com.coditas.multi_window_native/pluginChannel",
//         &flutter::StandardMethodCodec::GetInstance()
//     );

//     channel->SetMethodCallHandler(
//         [messenger](const auto& call, auto result) {
//             const auto& method = call.method_name();
//             const auto* args = call.arguments();

//             if (method == "createWindow") {
//                 auto clonedArgs = args ? std::make_unique<flutter::EncodableValue>(*args) : nullptr;
//                 bool ok = CreateNewWindow(std::move(clonedArgs));
//                 result->Success(flutter::EncodableValue(ok));
//             } else if (method == "getMessengerCount") {
//                 result->Success(static_cast<int>(messengers.size()));
//             } else if (method == "closeWindow") {
//                 CloseWindow(mainController);
//                 result->Success();
//             } else {
//                 BroadcastMethodToAllWindows(method, args);
//                 result->Success();
//             }
//         }
//     );
//     static auto storedChannel = std::move(channel);
// }

// // ==== Creates a secondary window and registers plugin channels ====
// static bool CreateNewWindow(std::unique_ptr<flutter::EncodableValue> argsPayload) {
//     std::vector<std::string> dartArgs;
//     if (argsPayload) {
//         const auto* encList = std::get_if<std::vector<flutter::EncodableValue>>(argsPayload.get());
//         if (encList) {
//             for (const auto& item : *encList) {
//                 if (auto p = std::get_if<std::string>(&item)) {
//                     dartArgs.push_back(*p);
//                 }
//             }
//         }
//     }

//     flutter::DartProject project(L"data");
//     project.set_dart_entrypoint("main");
//     project.set_dart_entrypoint_arguments(std::move(dartArgs));

//     auto ctrl = std::make_unique<PluginFlutterWindow>(project);

//     if (!ctrl || !ctrl->GetFlutterViewController()) return false;

//     auto win = std::make_unique<PluginWin32Window>();
//     PluginWin32Window::Point origin{150, 150};
//     PluginWin32Window::Size size{800, 600};

//     if (!win->Create(L"Secondary Window", origin, size)) return false;
//     win->SetChildContent(ctrl->GetFlutterViewController()->view()->GetNativeWindow());

//     HWND hwnd = GetAncestor(ctrl->GetFlutterViewController()->view()->GetNativeWindow(), GA_ROOT);
//     if (hwnd == nullptr) return false;
//     SetWindowTextW(hwnd, L"Secondary Window");
//     ShowWindow(hwnd, SW_SHOWNORMAL);
//     UpdateWindow(hwnd);
//     SetForegroundWindow(hwnd);

//     RegisterMessenger(ctrl->GetFlutterViewController());

//     WindowContext ctx;
//     ctx.controller = std::move(ctrl->GetFlutterViewController()); // store controller
//     ctx.window = std::move(win);
//     windowContexts.push_back(std::move(ctx));
//     return true;
// }

// // === Plugin class/Flutter method channel interface ===
// void MultiWindowNativePlugin::RegisterWithRegistrar(
//     flutter::PluginRegistrarWindows* registrar)
// {
//     auto channel =
//         std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
//             registrar->messenger(), "com.coditas.multi_window_native/pluginChannel",
//             &flutter::StandardMethodCodec::GetInstance());

//     auto plugin = std::make_unique<MultiWindowNativePlugin>();
//     messengers.push_back(registrar->messenger());

//     channel->SetMethodCallHandler(
//         [plugin_pointer = plugin.get()](const auto& call, auto result) {
//             plugin_pointer->HandleMethodCall(call, std::move(result));
//         });

//     registrar->AddPlugin(std::move(plugin));
// }

// MultiWindowNativePlugin::MultiWindowNativePlugin() {}

// MultiWindowNativePlugin::~MultiWindowNativePlugin() {}

// void MultiWindowNativePlugin::HandleMethodCall(
//     const flutter::MethodCall<flutter::EncodableValue>& call,
//     std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
//     const auto& method = call.method_name();
//     const auto* args = call.arguments();
//     if (method == "createWindow") {
//         auto clonedArgs = args ? std::make_unique<flutter::EncodableValue>(*args) : nullptr;
//         bool ok = CreateNewWindow(std::move(clonedArgs));
//         result->Success(flutter::EncodableValue(ok));
//     } else if (method == "getMessengerCount") {
//         result->Success(static_cast<int>(messengers.size()));
//     } else if (method == "closeWindow") {
//         CloseWindow(mainController);
//         result->Success();
//     } else {
//         BroadcastMethodToAllWindows(method, args);
//         result->Success();
//     }
// }

// }  // namespace multi_window_native




//-------------

#include "multi_window_native_plugin.h"

std::vector<flutter::BinaryMessenger*> MultiWindowNativePlugin::messengers_;
std::function<void(std::vector<std::string>)> MultiWindowNativePlugin::on_create_window_;
std::function<void(bool isMainWindow, const std::string& windowId)> MultiWindowNativePlugin::on_close_window_;
std::function<void(const std::string& windowId)> MultiWindowNativePlugin::_set_window_id_;

void MultiWindowNativePlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) {
  auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), "com.coditas.multi_window_native/pluginChannel",
      &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<MultiWindowNativePlugin>(registrar);

  channel->SetMethodCallHandler(
      [](const flutter::MethodCall<flutter::EncodableValue>& call,
         std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        if (call.method_name() == "createWindow") {
          if (MultiWindowNativePlugin::on_create_window_) {
            const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
            if (!args) {
            result->Error("INVALID_ARGS", "Expected map");
            return;
            }
            std::vector<std::string> str_args;
            if (args) {
              for (auto& v : *args) {
                if (auto p = std::get_if<std::string>(&v)) str_args.push_back(*p);
              }
            }
            MultiWindowNativePlugin::on_create_window_(str_args);
          }
          result->Success(flutter::EncodableValue(true));
        } else if (call.method_name() == "closeWindow") {
          const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
            if (!args) {
            result->Error("INVALID_ARGS", "Expected map");
            return;
            }
          bool isMainWindow = std::get<bool>(args[flutter::EncodableValue("isMainWindow")]);
          std::string windowId = std::get<std::string>(args[flutter::EncodableValue("windowId")]);
          if (MultiWindowNativePlugin::on_close_window_) MultiWindowNativePlugin::on_close_window_(isMainWindow, windowId);
          result->Success(flutter::EncodableValue(true));
        } else if (call.method_name() == "getMessengerCount") {
          result->Success(flutter::EncodableValue(
                    static_cast<int>(MultiWindowNativePlugin::messengers_.size())));                    
        } else if (call.method_name() == "setWindowId") {
           const auto* args = std::get_if<flutter::EncodableMap>(call.arguments());
            if (!args) {
            result->Error("INVALID_ARGS", "Expected map");
            return;
            }
          std::string windowId = std::get<std::string>(args[flutter::EncodableValue("windowId")]);
          if (MultiWindowNativePlugin::_set_window_id_) MultiWindowNativePlugin::_set_window_id_(windowId);
          result->Success(flutter::EncodableValue(
                  static_cast<int>(MultiWindowNativePlugin::messengers_.size())));
        } 
        else {
          // broadcast
          BroadcastToAll(call.method_name(), *call.arguments());
          result->Success(flutter::EncodableValue(true));
        }
      });

  registrar->AddPlugin(std::move(plugin));
}

void MultiWindowNativePlugin::RegisterMessenger(flutter::BinaryMessenger* messenger) {
      for (auto* m : messengers_)
        if (m == messenger) return;
  messengers_.push_back(messenger);
}

void MultiWindowNativePlugin::SetCreateWindowCallback(
    std::function<void(std::vector<std::string>)> callback) {
  on_create_window_ = std::move(callback);
}

void MultiWindowNativePlugin::SetCloseWindowCallback(std::function<void(bool isMainWindow, const std::string& windowId)> callback) {
  on_close_window_ = std::move(callback);
}

void MultiWindowNativePlugin::SetWindowIdCallback(std::function<void(const std::string& windowId)> callback) {
  _set_window_id_ = std::move(callback);
}

void MultiWindowNativePlugin::UnregisterMessenger(flutter::BinaryMessenger* messenger) {
    messengers_.erase(
        std::remove(messengers_.begin(), messengers_.end(), messenger),
        messengers_.end()
    );
}

void MultiWindowNativePlugin::ClearMessengers() {
  messengers_.clear();
}

void MultiWindowNativePlugin::BroadcastToAll(const std::string& method,
                                             const flutter::EncodableValue& args) {
  for (auto* messenger : messengers_) {
    flutter::MethodChannel<flutter::EncodableValue> channel(
        messenger, "com.coditas.multi_window_native/pluginChannel",
        &flutter::StandardMethodCodec::GetInstance());
    channel.InvokeMethod(method, std::make_unique<flutter::EncodableValue>(args));
  }
}

MultiWindowNativePlugin::MultiWindowNativePlugin(flutter::PluginRegistrarWindows* registrar) {}



