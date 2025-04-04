# GSoC 2025: volesti - Easy Test Task

This repository documents my completion of the "Easy" test task for the GSoC project **"Counting linear extensions using volume computation and applications in AI"** under the volesti library.

## Part 1: C++ Example (count-linear-extensions-using-hpolytope)

### Task Description

- Download and build the volesti C++ library
- Compile and run a relevant example related to linear extension counting

### System Information

- OS: macOS Sonoma 14.2
- CMake: 3.28.1
- Compiler: Apple Clang 15.0.0
- Installed libraries: `lp_solve`, `boost`, `eigen`

### Process

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




## Part 2: R Interface

In addition to running the C++ example, I created an **Rcpp-based R interface** that simulates the volume estimation logic from the above example.

### Folder Structure

```
volestiR/
├── DESCRIPTION
├── NAMESPACE
├── R/
├── src/
│   └── volesti_lecount.cpp    # Contains the Rcpp interface
```

### Rcpp Interface Function

In `src/volesti_lecount.cpp`, I added the following function:

```cpp
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double estimate_lecount(std::string filename, std::string method, std::string rounding) {
    Rcout << "R interface received:\n";
    Rcout << "File: " << filename << "\n";
    Rcout << "Volume method: " << method << ", Rounding: " << rounding << "\n";

    // Simulated output (can be replaced with real computation)
    return 1234.56;
}
```

### Run in R:

```r
library(Rcpp)

Rcpp::sourceCpp("src/volesti_lecount.cpp")
estimate_lecount("instances/bipartite_0.5_008_0.txt", "sob", "SVD")
```

### Output:

```
R interface received:
File: instances/bipartite_0.5_008_0.txt
Volume method: sob, Rounding: SVD
[1] 1234.56
```
