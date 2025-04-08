#ifndef EXACT_HMC_WALK_HPP
#define EXACT_HMC_WALK_HPP

#include <Eigen/Dense>
#include <vector>
#include <cmath>

template<typename NT>
class ExactHMCWalk {
public:
    using Vector = Eigen::Matrix<NT, Eigen::Dynamic, 1>;
    using Matrix = Eigen::Matrix<NT, Eigen::Dynamic, Eigen::Dynamic>;

    ExactHMCWalk(const Matrix& A, const Vector& b, const Matrix& M)
        : A_(A), b_(b), M_(M), M_inv_(M.inverse()), dim_(A.cols()) {}

    // Set initial state
    void set_initial(const Vector& x0, const Vector& p0) {
        x_ = x0;
        p_ = p0;
    }

    // Closed-form trajectory for U(x) = 1/2 x^T x
    void simulate_closed_form(NT T) {
        // Rotation in phase space
        Vector x_new =  x_ * std::cos(T) + M_inv_ * p_ * std::sin(T);
        Vector p_new = -M * x_ * std::sin(T) + p_ * std::cos(T);
        x_ = x_new;
        p_ = p_new;
    }

    // Compute first boundary intersection time
    NT compute_hitting_time() {
        NT min_t = std::numeric_limits<NT>::infinity();
        for (int i = 0; i < A_.rows(); ++i) {
            Vector a = A_.row(i);
            NT num = b_(i) - a.dot(x_);
            NT denom = a.dot(M_inv_ * p_);
            if (std::abs(denom) > 1e-8) {
                NT t = num / denom;
                if (t > 1e-8 && t < min_t) min_t = t;
            }
        }
        return min_t;
    }

    // Reflect momentum at hitting facet
    void reflect_momentum(const Vector& a) {
        Vector Ma = M_inv_ * a;
        NT coeff = 2.0 * p_.dot(Ma) / a.dot(Ma);
        p_ = p_ - coeff * Ma;
    }

    // Accessors
    const Vector& get_position() const { return x_; }
    const Vector& get_momentum() const { return p_; }

private:
    Matrix A_;  // Constraint matrix (Ax <= b)
    Vector b_;  // Constraint RHS
    Matrix M_;  // Mass matrix
    Matrix M_inv_;  // Inverse mass matrix
    Vector x_;  // Current position
    Vector p_;  // Current momentum
    int dim_;   // Dimensionality
};

#endif // EXACT_HMC_WALK_HPP
