import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'multi_window_native_method_channel.dart';

abstract class MultiWindowNativePlatform extends PlatformInterface {
  /// Constructs a MultiWindowNativePlatform.
  MultiWindowNativePlatform() : super(token: _token);

  static final Object _token = Object();

  static MultiWindowNativePlatform _instance = MethodChannelMultiWindowNative();

  /// The default instance of [MultiWindowNativePlatform] to use.
  ///
  /// Defaults to [MethodChannelMultiWindowNative].
  static MultiWindowNativePlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [MultiWindowNativePlatform] when
  /// they register themselves.
  static set instance(MultiWindowNativePlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  // Future<String?> getPlatformVersion() {
  //   throw UnimplementedError('platformVersion() has not been implemented.');
  // }

  Future<void> createAndRegisterWindow({
    required final String routeName,
    required final String theme,
    final String? argsJson,
    final void Function()? onCreation,
  });

  Future<void> notifyWindowClose();

  Future<int> getMessengerCount();

  Future<bool> notifyUiReady();

  Future<void> notifyAllWindows(String method, dynamic arguments);

  // Add other abstract methods corresponding to native features
}
