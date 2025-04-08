#include "ExactHMCWalk.hpp"
#include <iostream>

int main() {
    using NT = double;
    Eigen::MatrixXd A(4, 2);
    A << -1, 0,
          0, -1,
          1, 0,
          0, 1;
    Eigen::VectorXd b(4);
    b << 0, 0, 1, 1;

    Eigen::MatrixXd M = Eigen::Matrix2d::Identity();

    ExactHMCWalk<NT> walk(A, b, M);
    walk.set_initial(Eigen::Vector2d(0.5, 0.5), Eigen::Vector2d(1.0, 0.5));

    NT t_hit = walk.compute_hitting_time();
    std::cout << "First hitting time: " << t_hit << std::endl;

    walk.simulate_closed_form(t_hit);
    auto x_new = walk.get_position();
    std::cout << "New position after simulation:\n" << x_new << std::endl;

    // Reflect momentum at the boundary
    Eigen::Vector2d a = A.row(2);  // Assume this is the facet hit
    walk.reflect_momentum(a);
    auto p_new = walk.get_momentum();
    std::cout << "Reflected momentum:\n" << p_new << std::endl;

    return 0;
}
