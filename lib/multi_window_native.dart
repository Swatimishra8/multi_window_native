// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/to/pubspec-plugin-platforms.

import 'package:flutter/services.dart';
import 'package:multi_window_native/multi_window_native_method_channel.dart';

import 'multi_window_native_platform_interface.dart';



typedef MethodCallHandler = Future<dynamic> Function(MethodCall call);

class MultiWindowNative {

  factory MultiWindowNative() {
    return instance;
  }

  MultiWindowNative._internal();

  static final MultiWindowNative instance = MultiWindowNative._internal();

  // Future<String?> getPlatformVersion() {
  //   return MultiWindowNativePlatform.instance.getPlatformVersion();
  // }

  static void init() {
    MethodChannelMultiWindowNative.init();
  }

  static String registerListener(String methodName, MethodCallHandler handler) {
    return MethodChannelMultiWindowNative.registerListener(methodName, handler);
  }

  static void unregisterListener({required String methodName,required String id}) {
    MethodChannelMultiWindowNative.unregisterListener(methodName:  methodName, id:  id);
  }

  static Future<dynamic> handleMethodCall(MethodCall call) async {
    return MethodChannelMultiWindowNative.handleMethodCall(call);
  }

  static Future<void> createWindow(List<String> args) {
    return MultiWindowNativePlatform.instance.createAndRegisterWindow(
      routeName: args[0],
      theme: args[2],
      argsJson:  args[1],
    );
  }

  static Future<void> closeWindow() {
    return MultiWindowNativePlatform.instance.notifyWindowClose();
  }

  static Future<int> windowCount() {
    return MultiWindowNativePlatform.instance.getMessengerCount();
  }

  static Future<bool> notifyUiRendered() {
    return MultiWindowNativePlatform.instance.notifyUiReady();
  }

  static Future<void> notifyAllWindows(String method, dynamic arguments) async {
    return MultiWindowNativePlatform.instance.notifyAllWindows(method, arguments);
  }

  // Add other methods as needed
}
