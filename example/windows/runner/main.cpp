// #include <flutter/dart_project.h>
// #include <flutter/flutter_view_controller.h>
// #include <windows.h>

// #include "flutter_window.h"
// #include "utils.h"

// int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev,
//                       _In_ wchar_t *command_line, _In_ int show_command) {
//   // Attach to console when present (e.g., 'flutter run') or create a
//   // new console when running with a debugger.
//   if (!::AttachConsole(ATTACH_PARENT_PROCESS) && ::IsDebuggerPresent()) {
//     CreateAndAttachConsole();
//   }

//   // Initialize COM, so that it is available for use in the library and/or
//   // plugins.
//   ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

//   flutter::DartProject project(L"data");

//   std::vector<std::string> command_line_arguments =
//       GetCommandLineArguments();

//   project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

//   FlutterWindow window(project);
//   Win32Window::Point origin(10, 10);
//   Win32Window::Size size(1280, 720);
//   if (!window.Create(L"multi_window_native_example", origin, size)) {
//     return EXIT_FAILURE;
//   }
//   window.SetQuitOnClose(true);

//   ::MSG msg;
//   while (::GetMessage(&msg, nullptr, 0, 0)) {
//     ::TranslateMessage(&msg);
//     ::DispatchMessage(&msg);
//   }

//   ::CoUninitialize();
//   return EXIT_SUCCESS;
// }



//--------------------

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>
#include <windows.h>
// Import the generated plugin registrant
#include "generated_plugin_registrant.h"

#include "flutter_window.h"
#include "utils.h"
#include "win32_window.h"
#include "multi_window_native_plugin.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Context to store secondary windows
struct SecondaryWindowContext {
  std::string flutterWindowId; 
  std::unique_ptr<Win32Window> window;
  std::unique_ptr<flutter::FlutterViewController> controller;
};

static std::vector<std::unique_ptr<SecondaryWindowContext>> secondary_windows;
// Forward declare main window pointer
static Win32Window* main_window = nullptr;

// Function to create new secondary windows
void CreateNewWindow(const std::vector<std::string>& args) {
  flutter::DartProject project(L"data");
  project.set_dart_entrypoint("main");
  project.set_dart_entrypoint_arguments(args);

  auto flutter_controller = std::make_unique<flutter::FlutterViewController>(
      800, 600, project);

  if (!flutter_controller->engine() || !flutter_controller->view()) {
    std::cerr << "Failed to create FlutterViewController for secondary window"
              << std::endl;
    return;
  }

  // Register messenger with MultiWindow plugin
  MultiWindowNativePlugin::RegisterMessenger(
    flutter_controller->engine()->messenger());

  auto window = std::make_unique<Win32Window>();
  Win32Window::Point origin(50, 50);
  Win32Window::Size size(800, 600);

  if (!window->Create(L"Secondary Window", origin, size)) {
    std::cerr << "Failed to create secondary Win32 window" << std::endl;
    return;
  }

  //  Register all plugins for this engine
  RegisterPlugins(flutter_controller->engine());

  window->SetQuitOnClose(false);
  window->SetChildContent(flutter_controller->view()->GetNativeWindow());

  auto ctx = std::make_unique<SecondaryWindowContext>();
  ctx->window = std::move(window);
  ctx->controller = std::move(flutter_controller);

  secondary_windows.push_back(std::move(ctx));
}

// Function to close windows
void CloseWindow(bool isMainWindow,const std::string& windowId) {
  if (isMainWindow) {
      // Close all secondary windows and quit app
      for (auto& ctx : secondary_windows) {
          if (ctx.windowId != windowId) {  // skip main window itself
              HWND hwnd = ctx.window->GetHandle();
              if (hwnd) {
                  ::PostMessage(hwnd, WM_CLOSE, 0, 0);
              }
          }
      }
      secondary_windows.clear();
      messengers.clear();
      PostQuitMessage(0);  
  } else {
      auto it = std::find_if(secondary_windows.begin(), secondary_windows.end(),
                              [&windowId](const SecondaryWindowContext& ctx) { return ctx.windowId == windowId; });

      if (it != secondary_windows.end()) {
          auto& ctx = *it;
          // Shutdown engine and remove messenger
          auto engine = ctx.controller->engine();
          auto messengerPtr = engine->messenger();
          engine->Shutdown();
          messengers.erase(std::remove(messengers.begin(), messengers.end(), messengerPtr),
                            messengers.end());

          // Destroy native window
          HWND hwnd = GetAncestor(ctx.controller->view()->GetNativeWindow(), GA_ROOT);
          if (hwnd) {
              DestroyWindow(hwnd);
          }
          // Remove context
          secondary_windows.erase(it);
      }
  }
}

int APIENTRY wWinMain(HINSTANCE instance,
                      HINSTANCE prev,
                      wchar_t* command_line,
                      int show_command) {
  // Attach to console if available (flutter run) or create one for debugging
  if (!::AttachConsole(ATTACH_PARENT_PROCESS) && ::IsDebuggerPresent()) {
    CreateAndAttachConsole();
  }

  // Initialize COM
  ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  // Register callbacks for MultiWindow plugin
  MultiWindowNativePlugin::SetCreateWindowCallback(CreateNewWindow);
  MultiWindowNativePlugin::SetCloseWindowCallback(CloseWindow);
  MultiWindowNativePlugin::SetWindowIdCallback(
    [](const std::string& windowId) {
        // Update SecondaryWindowContext with this window ID
        if (!secondary_windows.empty()) {
            secondary_windows.back()->windowId = windowId;
        }
    });

  // Launch the main Flutter window
  flutter::DartProject project(L"data");
  std::vector<std::string> command_line_arguments = GetCommandLineArguments();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  FlutterWindow window(project);
  Win32Window::Point origin(10, 10);
  Win32Window::Size size(1280, 720);

  if (!window.Create(L"multi_window_native_example", origin, size)) {
    return EXIT_FAILURE;
  }

  window.SetQuitOnClose(false);

  // Run message loop
  ::MSG msg;
  while (::GetMessage(&msg, nullptr, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  ::CoUninitialize();
  return EXIT_SUCCESS;
}
