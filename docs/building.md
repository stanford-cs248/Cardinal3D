---
layout: default
title: "Building Cardinal3D"
permalink: /build/
---

# Building Cardinal3D

To create a private git mirror that can pull changes from Cardinal3D, see [Git Setup](git) (optional, you can just simply clone the repo).

Note: the first build on any platform will be very slow, as it must compile most dependencies. Subsequent builds will only need to re-compile your edited Cardinal3D code.

### Linux 

The following packages (ubuntu/debian) are required, as well as CMake and either gcc or clang:
```
sudo apt install pkg-config libgtk-3-dev libsdl2-dev
```

The version of CMake packaged with apt may be too old (we are using the latest version). If this is the case, you can install the latest version through pip:
```
pip install cmake
export PATH=$PATH:/usr/local/bin
```

Finally, to build the project:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. 
make -j4
```

The same process should also work modified for your distro/package manager of choice. Note that if you are using Wayland, you may need to set the environment variable ``SDL_VIDEODRIVER=wayland`` when running ``Cardinal3D`` for acceptable performance.

Notes:
- You can instead use ``cmake -DCMAKE_BUILD_TYPE=Debug ..`` to build in debug mode, which, while far slower, makes the debugging experience much more intuitive.
- You can replace ``4`` with the number of build processes to run in parallel (set to the number of cores in your machine for maximum utilization).
- If you have both gcc and clang installed and want to build with clang, you should run ``CC=clang CXX=clang++ cmake ..`` instead.
- Since our codebase uses new features in c++17, you need to have g++ version >= 7 to compile it. You can run ``CC=gcc-7 CXX=g++-7 cmake ..``. 

### Windows

The windows build is easiest to set up using the Visual Studio compiler (for now). To get the compiler, download and install the Visual Studio Community 2022 [here](https://visualstudio.microsoft.com/downloads/). Be sure to install the "Desktop development with C++" component. You can download CMake for windows [here](https://cmake.org/download/).

Once you have VS and CMake installed, you can follow the same procedure as PA1 (first run CMake to create Cardinal3D.sln, then open it in VS) to build, run and debug the project. The default build mode is ``RelWithDebInfo``, you can also build in ``Debug`` mode, which, while far slower, makes the debugging experience much more intuitive.

Alternatively you can simply run our provided script (double click) ``build_win.bat`` in the project root, which will automatically import the compiler and build the project for you without running CMake and VS GUI. ``Cardinal3D.exe`` will be generated under ``build/RelWithDebInfo/``. (You may need to edit the script if you installed a different version of VS or used custom install path.)

Otherwise, if you want to build manually, the steps (assuming MSVC is in scope) are:
```
mkdir build
cd build
cmake ..
cmake --build . --config RelWithDebInfo
```

<!-- Once the Visual Studio compiler (MSVC) is installed, you can access it by running "Developer Command Prompt for VS 2022," which opens a terminal with the utilities in scope. The compiler is called ``cl``. You can also import these utilities in any terminal session by running the script installed at ``C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat``. 

You can also use ``--config Debug`` to build in debug mode, which, while far slower, makes the debugging experience much more intuitive. If you swap this, be sure to make a new build directory for it.

Finally, also note that ``cmake ..`` generates a Visual Studio solution file in the current directory. You can open this solution (``Cardinal3D.sln``) in Visual Studio itself and use its interface to build, run, and debug the project. (Using the Visual Studio debugger or the provided VSCode launch options for debugging is highly recommended.) -->

### MacOS

The following packages are required, as well as CMake and clang. You can install them with [homebrew](https://brew.sh/):
```
brew install pkg-config sdl2
```

To build the project:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. 
make -j4
```

Notes:
- You can instead use ``cmake -DCMAKE_BUILD_TYPE=Debug ..`` to build in debug mode, which, while far slower, makes the debugging experience much more intuitive.
- You can replace ``4`` with the number of build processes to run in parallel (set to the number of cores in your machine for maximum utilization).

### Remote server
Currently, Cardinal3D can only be built on Myth server, due to the c++17 features (Rice server only supports g++-5). 
First install latest CMake locally and create an alias for it 
```
pip install cmake --user
alias cmake=~/.local/bin/cmake
```
Then create the build folder and run the following command to use g++-7.  
```
CC=gcc-7 CXX=g++-7 cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
``` 
