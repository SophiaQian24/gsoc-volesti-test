# GSoC 2025: volesti - Easy Test Task

This repository documents my completion of the "Easy" test task for the GSoC project **"Counting linear extensions using volume computation and applications in AI"** under the volesti library.

## Task Description

- Download and build the volesti C++ library
- Compile and run a relevant example related to linear extension counting

## System Information

- OS: macOS Sonoma 14.2
- CMake: 3.28.1
- Compiler: Apple Clang 15.0.0
- Installed libraries: `lp_solve`, `boost`, `eigen`

## What I Did

1. Cloned the official volesti repository
2. Located the `examples/count-linear-extensions-using-hpolytope` directory
3. Modified `LPSolve.cmake` and `CMakeLists.txt` to correctly link with the Homebrew-installed `lp_solve` library (since auto-download failed)
4. Built the project with CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
5. Successfully compiled the example program volesti_lecount.

Output: [100%] Built target volesti_lecount
