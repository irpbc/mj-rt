# The MJ language native libraries

This project contains C++ code required to run the MJ compiler and execute compiled programs.

## [The compiler helper code](compiler)

The MJ compiler makes use of some LLVM C++ APIs, which are not exposed as C functions, thus
requiring some C++ code, which is called via P/Invoke from C#.

## [MJ Runtime static library](premain_shim)

The runtime library requires some code that must be statically linked into every compiled
program. This code extracts the stack maps left by LLVM in a section of the executable and 
passes them to the runtime initialization routine.

On Mac OS we call a standard function for extracting data from the section. On Windows binary
info API is much more low level so we use some (heavily shrinked down) code from 
[changeofpace/x64dbg-Anti-Debug-POC](https://github.com/changeofpace/x64dbg-Anti-Debug-POC).

## [MJ Runtime dynamic library](runtime)

This is the MJ runtime proper. It contains an accurate compacting garbage collector.
The GC is triggered if an allocation cannot fit into available heap space. So it does not
support multi threaded programs, and it runs only when heap is exhausted. This makes it very 
small (~500 lines, including some, heavily modified, code from 
[kavon/llvm-statepoint-utils](https://github.com/kavon/llvm-statepoint-utils)). 
It reads LLVM stack maps, populates a hash table with return addresses as keys. When GC is
triggered it scans the stack for roots using this data structure.

The runtime library also contains all the built in functions of the language.

## Building this project

This project uses CMake for cross platform building. You can download the official CMake packages
for your OS. It includes a GUI for easy configuration.

### Windows

You must install Visual Studio 2017, because we use Microsoft dev tools (for now) to be most
compatible with Windows. 

LLVM does not support creation of `libLLVM.dll` on Windows, which is why LLVMSharp project 
builds its own dll from LLVM static libs and explicitly exports all C API symbols with 
a `.def` file. This project does the same thing.

Steps to build LLVM static libs:

* Clone [LLVM source code](https://github.com/llvm-mirror/llvm), switch to branch `release_50`. 
* Open CMake GUI. Select the LLVM source code folder.
* Create a folder in the source code folder and select that as the build folder.
* Hit Configure. Choose "Visual Studio 15 2017 Win64" generator and proceed.
* A table of options will be displayed. 
    * Set `CMAKE_INSTALL_PREFIX` to some convenient folder (eg. `D:/LLVMHome`).
    * Uncheck `LLVM_BUILD_TESTS`, `LLVM_BUILD_TOOLS` and `LLVM_BUILD_UTILS` because we won't need those.
    * Uncheck `LLVM_INCLUDE_TESTS`, `LLVM_INCLUDE_TOOLS` and `LLVM_INCLUDE_UTILS`, for same reason.
    * Set `LLVM_TARGETS_TO_BUILD` to `X86`.
* Hit Configure, than Generate.
* Open the generated solution in Visual Studio 2017
* Select the `Release` configuration
* Build the `BUILD_ALL` project, and then the `INSTALL` project.

The folder you selected earlier will now be populated with built libraries. Next, you must open Environment 
Variables editor. Insert a variable `CMAKE_PREFIX_PATH` with path to LLVM install folder as the value.
This will ensure LLVM libraries will be found by CMake in the next steps.
 
Steps to build this project:

* Open CMake GUI. Select this project's source code folder.
* Create a folder in the source code folder and select that as the build folder.
* Hit Configure. Choose "Visual Studio 15 2017 Win64" generator and proceed.
* Hit Configure, than Generate.
* Open the generated solution in Visual Studio 2017
* Select `Debug ` or `Release` configuration. (`libLLVM` must be built in the same configuration as
the LLVM libs were built, usually `Release`)
* Build the `BUILD_ALL` project.

The build will produce a lot of files. Following files should be copied to folder where you are running the
MJ compiler: `mj_rt.lib`, `mj_rt.dll`, `mj_shim.lib`, `libLLVM.dll`.

### Mac OS

You must have XCode installed so the C++ compiler and linker are available. On Mac OS LLVM does support
creation of libLLVM.dylib with all symbols already exported (which is part of LLVMSharp). We will build LLVM 
from source just for the purpose of producing a CMake package to link the compiler native code against.

You should add `/Application/CMake.app/Content/bin` to your `PATH` environment variable, because we will
build from command line.

Steps to build LLVM:

* Clone [LLVM source code](https://github.com/llvm-mirror/llvm), switch to branch `release_50`. 
* Open CMake GUI. Select the LLVM source code folder.
* Create a folder in the source code folder and select that as the build folder.
* Hit Configure. Choose "Unix Makefiles" generator and proceed.
* A table of options will be displayed. 
    * Set `CMAKE_BUILD_TYPE` to `Release`
    * Set `CMAKE_INSTALL_PREFIX` to some convenient folder (eg. `<some folder>/LLVMHome`).
    * Uncheck `LLVM_BUILD_TESTS`, `LLVM_BUILD_TOOLS` and `LLVM_BUILD_UTILS` because we won't need those.
    * Uncheck `LLVM_INCLUDE_TESTS`, `LLVM_INCLUDE_TOOLS` and `LLVM_INCLUDE_UTILS`, for same reason.
    * Set `LLVM_TARGETS_TO_BUILD` to `X86`.
    * Check `LLVM_BUILD_LLVM_DYLIB`. This option is what we actually need, all of the above is just to
    speed up the build.
* Hit Configure, than Generate.
* In Terminal `cd` into LLVM source dir and do:
    * `cmake --build ./<build dir> -- -j4` (replace `4` with number of virtual cores of your CPU, for
    multi core building)
    * `cmake --build ./<build dir> --target install`

The folder you selected earlier will now be populated with built libraries. Next, you must add an 
environment variable `CMAKE_PREFIX_PATH` with path to LLVM install folder as the value. This will ensure 
LLVM libraries will be found by CMake in the next steps.

Steps to build this project:

* Open CMake GUI. Select this project's source code folder.
* Create a folder in the source code folder and select that as the build folder.
* Hit Configure. Choose "Unix Makefiles" generator and proceed.
* Set `CMAKE_BUILD_TYPE` to `Release` or `Debug`
* Hit Configure, than Generate.
* In Terminal `cd` into this directory and do:
    * `cmake --build ./<build dir>`

The build will produce files: `libmj_rt.dylib`, `libmj_compiler_native.dylib` and `libmj_shim.a`. These
files should be copied over to the folder where you are running the MJ compiler.
