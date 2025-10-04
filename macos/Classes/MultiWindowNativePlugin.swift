import Cocoa
import FlutterMacOS

typealias SecondaryWindowControllers = (
    window: NSWindow, flutterViewController: FlutterViewController
)

public class MultiWindowNativePlugin: NSObject, FlutterPlugin,  NSWindowDelegate{

    // Callback to register plugins for new engines
    public static var onEngineCreatedCallback: ((FlutterEngine) -> Void)?

    private static var messengers: [FlutterBinaryMessenger] = []
    private  var secondaryWindowControllers: [SecondaryWindowControllers] = []
    private  var mainWindow: NSWindow?
    private static let channelName = "com.coditas.multi_window_native/pluginChannel"

    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: channelName, binaryMessenger: registrar.messenger)
        let instance = MultiWindowNativePlugin()
        registrar.addMethodCallDelegate(instance, channel: channel)
        
        // Setup main window reference and messenger
        if let mainFlutterWindow = NSApp.mainWindow,
           let controller = mainFlutterWindow.contentViewController as? FlutterViewController {
            let messenger = controller.engine.binaryMessenger
            messengers.append(messenger)
            print("messen=gers: main\(messengers.count)")
            instance.mainWindow = mainFlutterWindow
            mainFlutterWindow.delegate = instance
        }
    }

    
    // Handle method calls from Flutter (main window)
    public func handle(_ call: FlutterMethodCall,result: @escaping FlutterResult) {
        print("Inside handle")
        switch call.method {
        case "notifyUiReady":
            DispatchQueue.main.async {
            if let (window, _) = self.secondaryWindowControllers.last {
                window.makeKeyAndOrderFront(nil)
            }
                result(true)
            }
        case "createWindow":
            // guard let args = call.arguments as? [String] else {
            //     result(FlutterError(code: "ARG_ERROR", message: "Expected [String]", details: nil))
            //     return
            // }
            guard let args = call.arguments as? [String: Any] else {
                result(FlutterError(code: "ARG_ERROR", message: "Expected dictionary", details: nil))
                return
            }

            // Convert dictionary values to list of strings
            let argsList: [String] = [
                args["routeName"] as? String ?? "",
                args["theme"] as? String ?? "",
                args["argsJson"] as? String ?? ""
            ]
            createNewWindow(with: argsList) { success in result(success) }
        case "closeWindow":
            print("Inside close of handle")
            if let mainWindow = NSApp.mainWindow {
                self.closeWindow(mainWindow)
            }
          result(true)
        case "getMessengerCount":
            result(Self.messengers.count)
        default:
            // Broadcast other calls to all windows
            broadcastToAllWindows(method: call.method, arguments: call.arguments)
            result(true)
    }
    }
    
    
    // Setup channel for newly created window (especially for secondary)
    private func setupChannelHandler(for messenger: FlutterBinaryMessenger, controller: FlutterViewController) {
        print("inide setup  \(controller)")
        let channel = FlutterMethodChannel(name: "com.coditas.multi_window_native/pluginChannel", binaryMessenger: messenger)
        channel.setMethodCallHandler { [weak self] call, result in
         guard let self = self else { return }
        switch call.method {
            case "notifyUiReady":
            // Now safe to show window
                DispatchQueue.main.async {
                controller.view.window?.makeKeyAndOrderFront(nil)
                result(true)
                }
            case "createWindow":
                // guard let args = call.arguments as? [String] else {
                // result(FlutterError(code: "ARG_ERROR", message: "Expected [String]", details: nil))
                // return
                // }
                guard let args = call.arguments as? [String: Any] else {
                    result(FlutterError(code: "ARG_ERROR", message: "Expected dictionary", details: nil))
                    return
                }

                // Convert dictionary values to list of strings
                let argsList: [String] = [
                    args["routeName"] as? String ?? "",
                    args["theme"] as? String ?? "",
                    args["argsJson"] as? String ?? ""
                ]
                self.createNewWindow(with: argsList) { success in result(success) }
            case "closeWindow":
                print("Inside close of secondary \(controller.view.window!)")
                self.closeWindow(controller.view.window!)
                result(true)
            case "getMessengerCount":
                result(Self.messengers.count)
            default:
                // Broadcast to all windows for EVERY other method
                self.broadcastToAllWindows(method: call.method, arguments: call.arguments)
                result(true)
            }
        }
    }
    
    // Register messenger for new window
    private func registerMessenger(_ messenger: FlutterBinaryMessenger, controller: FlutterViewController) {
        if Self.messengers.contains(where: { $0 === messenger }) {
            return // Already registered
        }
        setupChannelHandler(for: messenger, controller: controller)
        Self.messengers.append(messenger)
        print("messen=gers: secodnary\(Self.messengers.count)")
    }
    
    // Create a new secondary window
    private  func createNewWindow(with args: [String], completion: @escaping (Bool) -> Void) {
        print("Creating new window with args: \(args)")
            let flutterProject = FlutterDartProject()
            flutterProject.dartEntrypointArguments = args
            let engine = FlutterEngine(name: "multi-window-engine-\(UUID().uuidString)", project: flutterProject)
            let controller = FlutterViewController(engine: engine, nibName: nil, bundle: nil)

            // Run engine and show window
            engine.run(withEntrypoint: "main")

             // ⚡ Call the callback to register all plugins
            MultiWindowNativePlugin.onEngineCreatedCallback?(engine)
            
            // Create the NSWindow
            let contentRect = NSMakeRect(0, 0, 1440, 900)
            let styleMask: NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable]
            let newWindow = NSWindow(contentRect: contentRect, styleMask: styleMask, backing: .buffered, defer: true)
            newWindow.setContentSize(NSSize(width: 1000, height: 1000))
            newWindow.minSize = NSSize(width: 800, height: 900)
            newWindow.title = "📌"
            newWindow.center()
            newWindow.delegate = self

            registerMessenger(engine.binaryMessenger, controller: controller)
           
            newWindow.contentViewController = controller
            secondaryWindowControllers.append((newWindow, controller))
    }
    
    // Close a window (handles both main and secondary windows)
    private func closeWindow(_ win: NSWindow) {
        print("inside close")
        if win == mainWindow {
            for (window, controller) in secondaryWindowControllers {
                let engine = controller.engine
                let messenger = engine.binaryMessenger
                
                engine.viewController = nil
                window.delegate = nil
                window.contentViewController = nil
                engine.shutDownEngine()
                Self.messengers.removeAll(where: { $0 === messenger })
                window.close()
            }
            secondaryWindowControllers.removeAll()
            Self.messengers.removeAll()
            NSApp.terminate(self)
        } else {
            if let index = secondaryWindowControllers.firstIndex(where: { $0.window == win }) {
                let controller = secondaryWindowControllers[index].flutterViewController
                let engine = controller.engine
                let messenger = engine.binaryMessenger
                let window = secondaryWindowControllers[index].window
                
                engine.viewController = nil
                window.delegate = nil
                window.contentViewController = nil
                engine.shutDownEngine()
                Self.messengers.removeAll(where: { $0 === messenger })
                window.close()
                secondaryWindowControllers.remove(at: index)
            }
        }
    }
    
    // Broadcast method calls to all window messengers
    private func broadcastToAllWindows(method: String, arguments: Any?) {
        print("messen=gers: total \(Self.messengers.count)")
        for messenger in Self.messengers {
            let channel = FlutterMethodChannel(name: "com.coditas.multi_window_native/pluginChannel", binaryMessenger: messenger) // Fix: Use Self.channelName
            channel.invokeMethod(method, arguments: arguments)
        }
    }
    
    // NSWindowDelegate method
    public func windowWillClose(_ notification: Notification) {
        if let window = notification.object as? NSWindow {
            closeWindow(window)
        }
    }
    
    // Prevent app from terminating after last window closes
    // public func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
    //     return true
    // }



}