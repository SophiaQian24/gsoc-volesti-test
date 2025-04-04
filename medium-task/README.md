# Medium Task

## Main Improvements

Some main improvements has already been implemented in issue # 179.

- **Return Type Change:**
  
  The rounding functions have been updated to return a tuple (e.g., `std::tuple<MT, VT, NT>`) instead of a pair. This change allows for a more flexible and comprehensive return of the ellipsoid parameters (the transformation matrix, the shift vector, and the scaling factor) without forcing a fixed structure.
  
- **Parameter Flexibility:**
  
  New parameters like `max_iterations` (which can be set by the user) and a maximum ratio (or sandwich ratio) have been introduced. This adjustment provides better control over the convergence criteria and numerical stability during the rounding process.
  
- **Multiple Rounding Methods:**
  
  Several rounding approaches have been implemented, including SVD rounding and min_sampling_covering_ellipsoid_rounding. These alternative methods have been tested and show consistent behavior on the same input data, suggesting they are robust against variations in the polytope geometry.

## Reproducing Issue #179

I attempted to reproduce [issue #179](https://github.com/GeomScale/volesti/issues/179) using two code files: `volesti_lecount_no_rounding.cpp` (which does not call any rounding method) and `volesti_lecount_rounding.cpp` (which calls a rounding method using max_inscribed_ellipsoid).

- `volesti_lecount_rounding.cpp` calls **max_inscribed_ellipsoid**, whereas `volesti_lecount_no_rounding.cpp` does not call a rounding method.
- I used the matrix data from `bayesiannetwork_andes_008_0.txt` for testing.

I ran the `volesti_lecount.cpp` from the volesti repository's `examples/count-linear-extensions-using-hpolytope/` folder. The SVD and MIN_ELLIPSOID methods have been implemented.

```bash
sophiaqian@chengxins-air volesti % time ./volesti_lecount examples/count-linear-extensions-using-hpolytope/instances/bayesiannetwork_andes_008_0.txt sob SVD
28.0422
./volesti_lecount  sob SVD  2.52s user 0.02s system 99% cpu 2.542 total
```
```bash
sophiaqian@chengxins-air volesti % time ./volesti_lecount examples/count-linear-extensions-using-hpolytope/instances/bayesiannetwork_andes_008_0.txt sob MIN_ELLIPSOID
28.0496
./volesti_lecount  sob MIN_ELLIPSOID  2.80s user 0.02s system 99% cpu 2.824 total
```

The results for volesti_lecount_no_rounding.cpp are as follows:
```bash
sophiaqian@chengxins-air count-linear-extensions-using-hpolytope % ./volesti_lecount_no_rounding instances/bayesiannetwork_andes_008_0.txt sob
30.7447
```
For volesti_lecount_rounding.cpp, the results are:
```bash
sophiaqian@chengxins-air count-linear-extensions-using-hpolytope % ./volesti_lecount_rounding instances/bayesiannetwork_andes_008_0.txt sob

Assertion failed: (dst.rows() == src.rows() && dst.cols() == src.cols()), function resize_if_allowed, file AssignEvaluator.h, line 754.
zsh: abort      ./volesti_lecount_rounding instances/bayesiannetwork_andes_008_0.txt sob
```
Through debugging, I discovered that the exception is thrown by max_inscribed_ellipsoid.

## Debugging the Issue
I carefully studied the code and, using debugging, found that the issue occurs in the following line:
```python
ATP_A.diagonal() += ones_m * reg;
```
The original code uses an m-dimensional ones vector (ones_m) to add a regularization term to the diagonal of ATP_A. However, ATP_A is an 
n×n
matrix, so its diagonal has only n elements. When 
m≠n,
using an m-dimensional vector leads to a dimension mismatch, triggering the assertion error.

The regularization operation needs to apply the regularization to each decision variable (n of them) rather than to each constraint (m of them). Therefore, an n-dimensional ones vector (ones_n) must be used instead.

I added:

```python
VT const bmAx0 = b - A * x0, ones_m = VT::Ones(m), ones_n = VT::Ones(n);
```
and modified the original line to:
```python
ATP_A.diagonal() += ones_n * reg;
```

After this change, the code runs normally:
```bash
sophiaqian@chengxins-air count-linear-extensions-using-hpolytope % ./volesti_lecount_rounding instances/bayesiannetwork_andes_008_0.txt sob
Matrix A dimensions: 24 x 8
Transformation matrix L dimensions: 8 x 8
Expected dimensions: 8 x 8
22.7084
sophiaqian@chengxins-air count-linear-extensions-using-hpolytope % ./volesti_lecount_rounding instances/bayesiannetwork_andes_008_0.txt sob
Matrix A dimensions: 24 x 8
Transformation matrix L dimensions: 8 x 8
Expected dimensions: 8 x 8
16.4434
sophiaqian@chengxins-air count-linear-extensions-using-hpolytope % ./volesti_lecount_rounding instances/bayesiannetwork_andes_008_0.txt sob
Matrix A dimensions: 24 x 8
Transformation matrix L dimensions: 8 x 8
Expected dimensions: 8 x 8
24.4268
```
However, the results are very unstable and require further research and testing with larger samples.

## Notes on Rounding Method
- **Max Inscribed Ellipsoid Rounding:**

Computes the largest inscribed ellipsoid, ensuring the ellipsoid lies entirely within P.

Pros: Theoretically provides the best rounding effect.
Cons: Computationally expensive and sensitive to degenerate polytopes, making it prone to dimension mismatch issues.

Cons: Computationally expensive and sensitive to degenerate polytopes, making it prone to dimension mismatch issues.

- **Minimum Sampling Covering Ellipsoid Rounding:**

Finds the smallest ellipsoid that covers P using a sampling-based estimation, which is simpler to compute.

Pros: More stable and can handle complex polytopes better.

Cons: Theoretically, it may not round P as precisely as the maximum inscribed ellipsoid.

- **SVD Rounding:**

Computes an affine transformation based on the covariance matrix of sample points from  P and uses SVD to construct the transformation.

Pros: Simple, intuitive, and numerically stable.

Cons: Relies on the representativeness of the sample and may be affected by sampling errors.