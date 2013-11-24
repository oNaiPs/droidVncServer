VNC Server for Google Glass
==========

## Why the fork?

- The project is based on [oNaips/droid-VNC-server](https://github.com/oNaiPs/droid-VNC-server) which is an VNC server implementation on Android
- The project is wrapped to be suitable for Google Glass using [GDK](https://developers.google.com/glass/develop/gdk/index)
- Some features incompatible with Glass is cut down, such as JNI binaries for other platforms

## Install

As GDK is still a Sneak Peek and distributing GDK Glassware via MyGlass is not supported yet, the only way to install is using ADB.

```
$ adb install glass-vnc-server.apk
```

## Usage

At the home screen, say "ok glass, start the VNC server".

## Structure

The projects consists in three main modules parts: the daemon, wrapper libs and the GUI.

### Daemon

Provides the vnc server functionality, injects input/touch events, clipboard management, etc.
Available in `jni/` folder.

### Wrapper libs

Compiled against the AOSP so everyone can build the daemon/GUI without having to fetch +2GB files.
Currently there are 2 wrappers, gralloc and flinger.

Available in `nativeMethods/` folder, and precompiled libs in `nativeMethods/lib/`.

### GUI

GUI handles user-friendly control.
Connects to the daemon using local IPC.

## Build

1. Compile JNI codes (daemon)

  ```
  $ cd <glass-vnc-server>
  $ ndk-build
  $ ./updateExecsAndLibs.sh
  ```

2. Compile wrapper libs (optional)

  ```
  $ cd <aosp_folder>
  $ . build/envsetup.sh
  $ lunch
  $ ln -s <glass-vnc-server>/nativeMethods/ external/
  $ cd external/nativeMethods
  $ mm .
  $ cd <droid-vnc-folder>
  $ ./updateExecsAndLibs.sh
  ```

3. Finally, build complete project

Import using eclipse as a regular Android project

## Roadmap

1. As I was addicted to Android Studio and not familiar to Eclipse, I'm planning to transform the whole project.
2. Maybe a Makefile for those tons of commands above?

## License

[GPL v3](LICENSE)
