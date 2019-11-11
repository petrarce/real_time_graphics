# Real-Time Graphics Framework Readme

## Description

This is the Real-Time Graphics Framework needed to build and run the exercise tasks.
It is designed for Windows and Linux. Mac might work as well, but is not tested or supported.

## Build Requirements

* CMake: <https://cmake.org/download/#latest>
* Linux: C++17 Compiler, e.g. GCC 8
* Windows: C++17 Compiler, e.g. Visual Studio 2017 Community: <https://www.visualstudio.com/de/downloads/>

## Build Instructions

### Linux (GUI)

We recommend using the QtCreator IDE <https://www.qt.io/download-qt-installer> and the newest GCC compiler
(Personally, we use zapcc as it is much faster, but you have to compile it yourself: <https://github.com/yrnkrn/zapcc>)
The CMake and build commands can be executed directly in the QtCreator IDE.

### Linux (Command-Line)

Building from the command line can be done (though is not recommended) via:

    mkdir build
    cd build
    cmake ..
    make -j`nproc`

(nproc returns the number of CPU threads available. Alternatively, you can execute e.g. make -j8)

### Windows (GUI)

Open CMake-Gui (cmake-gui.exe)
Set the source code path to where the `CMakeLists.txt` is (e.g. `C:/RTG/assignment01`).
Set the build path to where you want the Visual Studio project to be created and built (e.g. `C:/RTG/ass01-build`).
Click button "Configure" and choose e.g. `"Visual Studio 15 2017 Win64"` (The VS version must match the installed one and the Architecture (e.g. Win64) must match your computer's. Note that we do not support 32bit systems.).
Click button "Generate".
Open the Solution file (e.g. `assignment01.sln`) in Visual Studio. Set the build type to "Release" and click on "Build" -> "Build Solution" (or simply press F6) to build.

### Windows (Command-Line)

Open a command line window (cmd.exe or Windows Power Shell in Windows 10) and navigate to the folder that contains the `CMakeLists.txt` (e.g. `C:/RTG/assignment01`).

Create and go to build folder:

    md ass01-build
    cd ass01-build

Run cmake with root folder:

    cmake ..

Compile program in release:

    cmake --build . --config Release

### Release vs. Debug Builds on Windows

In general, Release builds should be sufficient to accomplish the tasks for the RTG assignments. However, if you really have to do some severe debugging, you might want to have debug builds.

Note: It is very important to keep the `CMAKE_BUILD_TYPE`-variable and the actual build config in Visual Studio consistent.
If you want a Release build, set `CMAKE_BUILD_TYPE` to Release in the CMake-Gui or -for the command-line way- replace the aforementioned cmake-call `cmake ..` with `cmake .. -DCMAKE_BUILD_TYPE=Release`. (Release is the default anyway)
For building the solution set the Build Type in Visual Studio to "Release" or use the command-line call mentioned above: `cmake --build . --config Release`

For the debug case, do the very same, but with "Debug" instead of "Release".

## Help / Feedback

If you need help building the framework or want to provide feedback you can get in touch in the following ways:

* Write a post in the RWTHmoodle discussion forum. This is especially recommended for all kinds of build problems. On the one hand other students can help you and can often reply faster than us. On the other hand all students benefit from the provided troubleshooting.
* Write an email to rtg@cs.rwth-aachen.de
