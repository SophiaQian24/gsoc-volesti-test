// VolEsti (volume computation and sampling library)

// Copyright (c) 2012-2024 Vissarion Fisikopoulos
// Copyright (c) 2018-2024 Apostolos Chalkis
// Copyright (c) 2021 Vaibhav Thakkar

//Contributed and/or modified by Alexandros Manochis, as part of Google Summer of Code 2020 program.
//Contributed and/or modified by Vaibhav Thakkar, as part of Google Summer of Code 2021 program.

// Licensed under GNU LGPL.3, see LICENCE file


#ifndef MVE_COMPUTATION_HPP
#define MVE_COMPUTATION_HPP

#include <utility>
#include <Eigen/Eigen>
#include "preprocess/rounding_util_functions.hpp"


/*
    Implementation of the interior point method to compute the largest inscribed ellipsoid in a
    given convex polytope by "Yin Zhang, An Interior-Point Algorithm for the Maximum-Volume Ellipsoid
    Problem (1999)".

    This C++ implementation is based on the Matlab implementation in https://github.com/Bounciness/Volume-and-Sampling/blob/1c7adfb46c2c01037e625db76ff00e73616441d4/external/mve11/mve_cobra/mve_solver_cobra.m

    The implmentation computes the largest inscribed ellipsoid {x | x = y + Es, ||s|| = 1}

    Input: matrix A, vector b such that the polytope P = {x | Ax<=b}
           interior point x0
           tolerance parameters tol, reg

    Output: center of the ellipsoid y
            matrix E2^{-1} = E_transpose * E
*/

// Using MT as to deal with both dense and sparse matrices, MT_dense will be the type of result matrix
template <typename MT_dense, typename MT, typename VT, typename NT>
std::tuple<MT_dense, VT, bool> max_inscribed_ellipsoid(MT A, VT b, VT const& x0,
                                                       unsigned int const& maxiter,
                                                       NT const& tol, NT const& reg)
{
    typedef Eigen::DiagonalMatrix<NT, Eigen::Dynamic> Diagonal_MT;
    //typedef matrix_computational_operator<MT> mat_op;

    int m = A.rows(), n = A.cols();
    bool converged = false;

    NT bnrm = b.norm(),
       last_r1 = std::numeric_limits<NT>::lowest(),
       last_r2 = std::numeric_limits<NT>::lowest(),
       prev_obj = std::numeric_limits<NT>::lowest(),
       gap, rmu, res, objval, r1, r2 ,r3, astep, ax,
       ay, az, tau, logdetE2;

    NT const reg_lim = std::pow(10.0, -10.0), tau0 = 0.75, minmu = std::pow(10.0, -8.0);

    NT *vec_iter1, *vec_iter2, *vec_iter3;

    VT x = VT::Zero(n), y = VT::Ones(m), bmAx = VT::Ones(m),
       h(m), z(m), yz(m), yh(m), R1(n), R2(m), R3(m), y2h(m), y2h_z(m), h_z(m),
       R3Dy(m), R23(m), dx(n), Adx(m), dyDy(m), dy(m), dz(m);

    VT const bmAx0 = b - A * x0, ones_m = VT::Ones(m), ones_n = VT::Ones(n);

    MT_dense Q(m,m), YQ(m,m), G(m,m), T(m,n), ATP(n,m), ATP_A(n,n);
    Diagonal_MT Y(m);
    MT YA(m, n);

    A = (ones_m.cwiseProduct(bmAx0.cwiseInverse())).asDiagonal() * A, b = ones_m;
    MT A_trans = A.transpose(), E2(n,n);

    auto llt = initialize_chol<NT>(A_trans, A);

    int i = 1;
    while (i <= maxiter) {

        Y = y.asDiagonal();

        update_Atrans_Diag_A<NT>(E2, A_trans, A, Y);
        Q.noalias() = A * solve_mat(llt, E2, A_trans, logdetE2);
        h = Q.diagonal();
        h = h.cwiseSqrt();

        if (i == 1) {
            // perform those computations only during the first iteration
            NT t = bmAx.cwiseProduct(h.cwiseInverse()).minCoeff();
            y *= (1.0 / (t * t));
            h *= t;
            vec_iter1 = bmAx.data();
            vec_iter2 = h.data();
            vec_iter3 = z.data();
            for (int j = 0; j < m; ++j) {
                *vec_iter3 = std::max(0.1, (*vec_iter1 - (*vec_iter2)));
                vec_iter1++;
                vec_iter2++;
                vec_iter3++;
            }
            Q *= (t * t);
            Y = Y * (1.0 / (t * t));
        }

        yz = y.cwiseProduct(z);
        yh = y.cwiseProduct(h);

        gap = yz.sum() / NT(m); // compute the gap between primal and dual solution
        rmu = std::min(0.5, gap) * gap;
        rmu = std::max(rmu, minmu);

        R1.noalias() = - A_trans * yh;
        R2 = bmAx - h - z;
        R3.noalias() = rmu * ones_m - yz;

        r1 = R1.template lpNorm<Eigen::Infinity>();
        r2 = R2.template lpNorm<Eigen::Infinity>();
        r3 = R3.template lpNorm<Eigen::Infinity>();

        res = std::max(r1, r2);
        res = std::max(res, r3);
        objval = logdetE2; //logdet of E2 is already divided by 2

        if (i % 10 == 0) {
            
            NT rel, Rel;
            
            // computing eigenvalues of E2
            auto op = get_mat_prod_op<NT>(E2);
            auto eigs = get_eigs_solver<NT>(op, n);
            eigs->init();
            int nconv = eigs->compute();
            if (eigs->info() == Spectra::COMPUTATION_INFO::SUCCESSFUL) {
                Rel = 1.0 / eigs->eigenvalues().coeff(1);
                rel = 1.0 / eigs->eigenvalues().coeff(0);
            } else {
                Eigen::SelfAdjointEigenSolver<MT> eigensolver(E2); // E2 is positive definite matrix
                if (eigensolver.info() == Eigen::ComputationInfo::Success) {
                    Rel = 1.0 / eigensolver.eigenvalues().coeff(0);
                    rel = 1.0 / eigensolver.eigenvalues().template tail<1>().value();
                } else {
                    std::runtime_error("Computations failed.");
                }
            }

            if (std::abs((last_r1 - r1) / std::min(NT(std::abs(last_r1)), NT(std::abs(r1)))) < 0.01 &&
                std::abs((last_r2 - r2) / std::min(NT(abs(last_r2)), NT(std::abs(r2)))) < 0.01 &&
                Rel / rel > 100.0 &&
                reg > reg_lim) {
                
                converged = false;
                //Stopped making progress
                break;
            }
            last_r2 = r2;
            last_r1 = r1;
        }

        // stopping criterion
        if ((res < tol * (1.0 + bnrm) && rmu <= minmu) ||
            (i > 1 && prev_obj != std::numeric_limits<NT>::lowest() &&
            (std::abs(objval - prev_obj) <= tol * std::min(std::abs(objval), std::abs(prev_obj)) ||
             std::abs(prev_obj - objval) <= tol) ) ) {
            
            //converged
            x += x0;
            converged = true;
            break;
        }

        prev_obj = objval; // storing the objective value of the previous iteration
        YQ.noalias() = Y * Q;
        G = YQ.cwiseProduct(YQ.transpose());
        y2h = 2.0 * yh;
        update_Diag_A<NT>(YA, Y, A); // YA = Y * A;

        vec_iter1 = y2h.data();
        vec_iter2 = z.data();
        vec_iter3 = y2h_z.data();
        for (int j = 0; j < m; ++j) {
            *vec_iter3 = std::max(reg, (*vec_iter1) * (*vec_iter2));
            vec_iter1++;
            vec_iter2++;
            vec_iter3++;
        }

        G.diagonal() += y2h_z;
        h_z = h + z;
        Eigen::PartialPivLU<MT_dense> luG(G);
        T.noalias() = luG.solve(MT_dense(h_z.asDiagonal()*YA));

        ATP.noalias() = MT_dense(y2h.asDiagonal()*T - YA).transpose();

        vec_iter1 = R3.data();
        vec_iter2 = y.data();
        vec_iter3 = R3Dy.data();
        for (int j = 0; j < m; ++j) {
            *vec_iter3 = (*vec_iter1) / (*vec_iter2);
            vec_iter1++;
            vec_iter2++;
            vec_iter3++;
        }
        R23 = R2 - R3Dy;
        ATP_A.noalias() = ATP * A;
        ATP_A.diagonal() += ones_n * reg;
        
        dx = ATP_A.lu().solve(R1 + ATP * R23); // predictor step
        // corrector and combined step & length
        Adx.noalias() = A * dx;
        dyDy = luG.solve(y2h.cwiseProduct(Adx-R23));
        dy = y.cwiseProduct(dyDy);
        dz = R3Dy - z.cwiseProduct(dyDy);

        vec_iter1 = Adx.data();
        vec_iter2 = bmAx.data();
        ax = -0.5;
        for (int j = 0; j < m; ++j) {
            ax = std::min(ax, - (*vec_iter1) / (*vec_iter2));
            vec_iter1++;
            vec_iter2++;
        }

        ax = -1.0 / ax;
        ay = -1.0 / std::min(dyDy.minCoeff(), -0.5);

        vec_iter1 = dz.data();
        vec_iter2 = z.data();
        az = -0.5;
        for (int j = 0; j < m; ++j) {
            az = std::min(az, (*vec_iter1) / (*vec_iter2));
            vec_iter1++;
            vec_iter2++;
        }

        az = -1.0 / az;
        tau = std::max(tau0, 1.0 - res);
        astep = tau * std::min(std::min(1.0, ax), std::min(ay, az)); // compute the step

        // update iterates
        x += astep * dx;
        y += astep * dy;
        z += astep * dz;

        bmAx -= astep * Adx;

        i++;
    }

    if (!converged) {
        x += x0;
    }

    return std::make_tuple(E2, x, converged);
}


#endif

