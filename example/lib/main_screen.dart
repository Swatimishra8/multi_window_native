import 'package:flutter/material.dart';
import 'package:multi_window_native/multi_window_native.dart';
import 'package:window_manager/window_manager.dart';

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> with WindowListener {

  final TextEditingController _controller = TextEditingController();
  String? _listenerId;
  ThemeMode _themeMode = ThemeMode.light;

  Future<void> createNewWindow() async {
    await MultiWindowNative.createWindow([
      'secondScreen', 
      '{}',    
      _themeMode.name, 
    ]);
  }
  
  @override
  void initState() {
    super.initState();
    windowManager.addListener(this);

    MultiWindowNative.registerListener("updateTheme", (call) async {
      debugPrint("inside main theme");
      setState(() {
        _themeMode =
            (call.arguments == "dark") ? ThemeMode.dark : ThemeMode.light;
      });
    });

    _listenerId =  MultiWindowNative.registerListener("updateText", (call)async{
      setState(() {
        _controller.text = call.arguments as String;
      });
  }
    );
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      await WidgetsBinding.instance.endOfFrame;
      await  MultiWindowNative.notifyUiRendered();
    });
  }


  @override
  void dispose() {
    MultiWindowNative.unregisterListener(methodName:  "updateText", id: _listenerId!);
    windowManager.removeListener(this);
    super.dispose();
  }

  @override
  Future<void> onWindowClose() async {
    debugPrint("Window to be dleted");
    await MultiWindowNative.closeWindow();
  }

  Future<void> _toggleTheme() async {
    final newTheme = _themeMode == ThemeMode.light ? "dark" : "light";
    await MultiWindowNative.notifyAllWindows("updateTheme", newTheme);
  }

  @override
  Widget build(BuildContext context) {
      final isDark = _themeMode == ThemeMode.dark;

    return MaterialApp(
       themeMode: _themeMode,
      darkTheme: ThemeData.dark(),
      theme: ThemeData.light(),
      home: Scaffold(
          appBar: AppBar(
            title: const Text('Multi Window Native Plugin Example'),
          ),
          body: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
              Text('MAIN WINDOW'),
              SizedBox(height: 5),
              TextField(decoration: InputDecoration(), controller: _controller),
              SizedBox(height: 5),
              ElevatedButton(
                onPressed: () async {
                  final text = _controller.text;
                  // Send text to native
                  await MultiWindowNative.notifyAllWindows("updateText", text);
                },
                child: Text('Pass args to new window'),
              ),
              SizedBox(height: 5),
              ElevatedButton(
                onPressed: () async {
                  await MultiWindowNative.createWindow([
                    'secondScreen',
                    '{}',
                    'light'
                  ]);
                },
                child: Text('New window'),
              ),
              SizedBox(height: 5),
              ElevatedButton(
                  onPressed: _toggleTheme,
                  child: Text("Switch to ${isDark ? "Light" : "Dark"} Theme"),
                ),
              ],
            ),
          ),
        ),
    );
  }
}