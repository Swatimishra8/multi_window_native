multi_window_native
===================

A Flutter plugin that enables native multi-window support on macOS.
It allows you to create and manage multiple Flutter windows, communicate between them, and synchronize UI state across windows.

---------------------------------------------------
✨ Features
---------------------------------------------------
- Create new secondary Flutter windows from the main app.
- Broadcast messages between windows using method channels.
- Pass theme & route arguments when creating windows.
- Listen for updates via Dart-side listeners (registerListener / unregisterListener).
- Automatically registers plugins for each new window engine.
- Native macOS implementation with seamless Dart integration.

---------------------------------------------------
🚀 Installation
---------------------------------------------------
Add to your pubspec.yaml:

dependencies:
  multi_window_native: ^0.0.1

Then run:
flutter pub get

---------------------------------------------------
🛠️ Setup (macOS)
---------------------------------------------------
No extra setup required. The plugin will automatically:
- Register plugins for newly created window engines.
- Keep track of all FlutterBinaryMessenger instances for broadcasting.

---------------------------------------------------
📖 Usage
---------------------------------------------------

1. Import the package:
import 'package:multi_window_native/multi_window_native.dart';

2. Create a new window:
final _multiWindowNative = MultiWindowNative();

Future<void> _openWindow() async {
  await _multiWindowNative.createWindow([
    'secondScreen', // Route name
    '{}',           // Arguments as JSON string
    'light'         // Theme mode
  ]);
}

3. Notify native when UI is ready:
@override
void initState() {
  super.initState();
  WidgetsBinding.instance.addPostFrameCallback((_) async {
    await WidgetsBinding.instance.endOfFrame;
    await _multiWindowNative.notifyUiRendered();
  });
}

4. Communicate between windows:

Send updates from Dart to native (and broadcast to all windows):
await _multiWindowNative.notifyAllWindows(
  "updateText",
  {"message": "Hello from Main Window"},
);

Listen for updates in each window:
late String _listenerId;
String _text = "";

@override
void initState() {
  super.initState();
  _listenerId = MultiWindowNative.registerListener("updateText", (call) async {
    setState(() {
      _text = (call.arguments as Map)['message'] ?? "";
    });
  });
}

@override
void dispose() {
  MultiWindowNative.unregisterListener(methodName: "updateText", id: _listenerId);
  super.dispose();
}

5. Example: Two windows updating text:

Main Window:
ElevatedButton(
  onPressed: () async {
    await _multiWindowNative.notifyAllWindows(
      "updateText",
      {"message": "Text from Main Window"},
    );
  },
  child: Text("Broadcast Text"),
)

Secondary Window:
Text("Received: $_text"),

Both windows will receive updates when one broadcasts.

---------------------------------------------------
📊 API Reference
---------------------------------------------------
- createWindow(List<String> args): Creates a new Flutter window with route, arguments, and theme.
- closeWindow(): Closes the current window.
- notifyUiRendered(): Informs native that the window is ready to display.
- notifyAllWindows(String method, dynamic arguments): Broadcasts a method call to all windows.
- registerListener(String method, MethodCallHandler handler): Registers a listener for method calls from native. Returns an id.
- unregisterListener({required String methodName, required String id}): Unregisters a specific listener by method and id.

---------------------------------------------------
📌 Notes
---------------------------------------------------
- Supports macOS and Windows Both.
- Linux support may be added in future releases.
- Each secondary window runs its own Flutter engine.

---------------------------------------------------
📄 License
---------------------------------------------------
MIT License. See LICENSE for details.
