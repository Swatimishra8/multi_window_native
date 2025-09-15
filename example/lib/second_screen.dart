import 'package:flutter/material.dart';
import 'package:multi_window_native/multi_window_native.dart';
import 'package:window_manager/window_manager.dart';

class SecondScreen extends StatefulWidget {
  const SecondScreen({super.key});

  @override
  State<SecondScreen> createState() => _SecondScreenState();
}

class _SecondScreenState extends State<SecondScreen> with WindowListener{

  String _text = "";
  String? _listenerId;
  late String _themeListenerId;
  ThemeMode _themeMode = ThemeMode.light;
  
  @override
  void initState() {
    super.initState();
    windowManager.addListener(this);
    print("inside init");
     // Register listener and store the returned ID
    _listenerId =  MultiWindowNative.registerListener("updateText", (call) async {
      setState(() {
        _text = call.arguments as String;
      });
    },);
    _themeListenerId =
        MultiWindowNative.registerListener("updateTheme", (call) async {
             debugPrint("inside second theme");
      setState(() {
        _themeMode =
            (call.arguments == "dark") ? ThemeMode.dark : ThemeMode.light;
      });
    });
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      await WidgetsBinding.instance.endOfFrame;
      await  MultiWindowNative.notifyUiRendered();
    });
  }

  @override
  void dispose() {
    if (_listenerId != null) {
      MultiWindowNative.unregisterListener(methodName:  "updateText",id:  _listenerId!);
    }
    MultiWindowNative.unregisterListener(
        methodName: "updateTheme", id: _themeListenerId);
    windowManager.removeListener(this);
    super.dispose();
  }

  @override
  Future<void> onWindowClose() async {
    debugPrint("Window to be dleted ${await windowManager.getId()}");
    await MultiWindowNative.closeWindow();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Multi Window Demo',
      themeMode: _themeMode,
      darkTheme: ThemeData.dark(),
      theme: ThemeData.light(),
      home: Center(
        child: Scaffold(
            appBar: AppBar(
              title: const Text('Multi Window Native Plugin Example'),
            ),
            body: Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text("updated text$_text", style: const TextStyle(fontSize: 24)),
                ],
              ),
            ),
          ),
      ),
    );
  }
}