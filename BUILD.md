# Building droid-VNC-server

This document provides instructions for building the droid-VNC-server project.

## Prerequisites

### Required Tools

1. **Android NDK**: Version r21e or compatible
   - Download from: https://developer.android.com/ndk/downloads
   - Or install via package manager on Ubuntu:
     ```bash
     sudo apt-get install google-android-ndk-r21e-installer
     ```

2. **Java Development Kit (JDK)**: Version 8 or higher
   - For the Android GUI component

3. **Make**: Usually pre-installed on Linux systems

## Project Structure

The droid-VNC-server project consists of three main components:

### 1. Daemon (Native C code)
- Location: `jni/` folder
- Provides VNC server functionality, input/touch event injection, clipboard management
- Built using Android NDK

### 2. Wrapper Libraries
- Location: `nativeMethods/` folder
- Precompiled wrappers: `nativeMethods/lib/`
- Includes gralloc and flinger wrappers compiled against AOSP

### 3. GUI (Android Application)
- User-friendly control interface
- Connects to the daemon using local IPC

## Build Instructions

### Building the C Daemon

1. Navigate to the project root directory:
   ```bash
   cd /path/to/droidVncServer
   ```

2. Build using NDK:
   ```bash
   ndk-build
   ```

   If `ndk-build` is not in your PATH, use the full path:
   ```bash
   $ANDROID_NDK_HOME/ndk-build
   ```

3. Update executables and libraries:
   ```bash
   ./updateExecsAndLibs.sh
   ```

   This script will:
   - Rename `androidvncserver` executables to `libandroidvncserver.so`
   - Copy native method libraries from `nativeMethods/libs/` to `libs/`

### Building Wrapper Libraries (Optional)

**Note**: This requires the full Android Open Source Project (AOSP) and is not necessary for most builds since precompiled libraries are included.

1. Setup AOSP environment:
   ```bash
   cd <aosp_folder>
   . build/envsetup.sh
   lunch
   ```

2. Link native methods:
   ```bash
   ln -s <droid-vnc-folder>/nativeMethods/ external/
   ```

3. Build wrappers:
   ```bash
   cd external/nativeMethods
   mm .
   ```

4. Update libraries:
   ```bash
   cd <droid-vnc-folder>
   ./updateExecsAndLibs.sh
   ```

### Building the GUI

Import the project into Android Studio or Eclipse as a regular Android project and build.

## Continuous Integration

This project includes a GitHub Actions workflow (`.github/workflows/build.yml`) that automatically:
- Sets up the Android NDK
- Builds the native code
- Runs tests
- Uploads build artifacts

To trigger a build, simply push to any branch or create a pull request.

## Supported Architectures

The project builds for the following Android ABIs (defined in `jni/Application.mk`):
- armeabi
- armeabi-v7a
- x86
- mips

## Troubleshooting

### NDK not found
- Ensure the Android NDK is installed
- Set the `ANDROID_NDK_HOME` environment variable:
  ```bash
  export ANDROID_NDK_HOME=/path/to/android-ndk
  export PATH=$PATH:$ANDROID_NDK_HOME
  ```

### Build errors
- Make sure all dependencies are installed
- Check that you're using a compatible NDK version (r21e recommended)
- Verify that `jni/Android.mk` and `jni/Application.mk` are present

### Network issues during NDK installation
- Download the NDK manually from the Android developer website
- Extract to a local directory
- Set `ANDROID_NDK_HOME` to point to the extracted directory

## Build Output

After a successful build, you'll find:
- Native libraries in `libs/<architecture>/`
- The main executable renamed as `libandroidvncserver.so`
- Wrapper libraries copied from `nativeMethods/libs/`

## Cleaning the Build

To clean build artifacts:
```bash
ndk-build clean
rm -rf libs/armeabi* libs/x86 libs/mips*
```

## Additional Information

For more details about the project, see the [README](README) file.
