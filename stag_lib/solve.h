//
// Methods for solving Laplacian systems of equations.
//
// This file is provided as part of the STAG library and released under the MIT
// license.
//

/**
 * @file solve.h
 * \brief Methods for solving Laplacian systems of equations.
 */

#ifndef STAG_TEST_SOLVE_H
#define STAG_TEST_SOLVE_H

#include <stdexcept>
#include "graph.h"

/**
 * \cond do not document the STAG_MAX_ITERATIONS definition
 */
// Define a default maximum number of iterations for iterative methods.
#define STAG_MAX_ITERATIONS 1000

/**
 * \endcond
 */

namespace stag {

  /**
   * Solve the Laplacian system \f$L x = b\f$, where \f$L\f$ is the Laplacian
   * of the provided STAG graph object.
   *
   * The method for solving the system is chosen automatically by STAG.
   *
   * @param g
   * @param b
   * @param eps
   * @return
   */
  DenseVec solve_laplacian(Graph* g, DenseVec& b, double eps);

  /**
   * Solve the Laplacian system \f$L x = b\f$ by Jacobi iteration.
   *
   * At each iteration, we solve the system
   *
   * \f[
   *    P x_{k+1} = (P - L) x_k + b,
   * \f]
   *
   * where \f$P = \mathrm{diag}(L)\f$ is the diagonal of the Laplacian matrix
   * \f$L\f$.
   * The error at iteration \f$k\f$ is given by
   *
   * \f[
   *    e_k = \| L x_k - b \|_2,
   * \f]
   *
   * and we terminate the algorithm when \f$e_k\f$ is less than the provided
   * error parameter.
   *
   * \note
   * Jacobi iteration is guaranteed to converge if the Laplacian matrix is
   * Strictly Diagonally Dominant (SDD), and may also converge in other cases.
   *
   * @param g the graph representing the Laplacian matrix to be used
   * @param b the vector \f$b\f$
   * @param eps the error parameter \f$\epsilon\f$ controlling the permitted
   *            approximation error.
   * @param max_iterations (optional) the maximum number of iterations to perform.
   *                       If this parameter is omitted, STAG will automatically
   *                       set the maximum iterations.
   * @return the approximate solution \f$\hat{x}\f$ such that
   *         \f$\| L \hat{x} - b \|_2 \leq \epsilon\f$.
   * @throws stag::ConvergenceError if the algorithm does not converge
   */
  DenseVec solve_laplacian_jacobi(Graph* g, DenseVec& b, double eps,
                                  stag_int max_iterations);

  /**
   * @overload
   */
  DenseVec solve_laplacian_jacobi(Graph* g, DenseVec& b, double eps);

  /**
   * Solve a linear system \f$A x = b\f$ by Jacobi iteration.
   *
   * For more information about Jacobi iteration, see
   * stag::solve_laplacian_jacobi.
   *
   * @param A the matrix \f$A\f$
   * @param b the vector \f$b\f$
   * @param eps the error parameter \f$\epsilon\f$ controlling the permitted
   *            approximation error
   * @param max_iterations the maximum number of iterations to perform
   * @return the approximate solution \f$\hat{x}\f$ such that
   *         \f$\| A \hat{x} - b \|_2 \leq \epsilon\f$.
   * @throws stag::ConvergenceError if the algorithm does not converge
   */
  DenseVec jacobi_iteration(const SprsMat* A, DenseVec& b, double eps,
                            stag_int max_iterations);


  /**
   * Solve the Laplacian system \f$L x = b\f$ by the Gauss-Seidel method.
   *
   * At each iteration, we solve the system
   *
   * \f[
   *    P x_{k+1} = (P - L) x_k + b,
   * \f]
   *
   * where \f$P = \mathrm{lower}(L)\f$ is the lower-triangular part of
   * the Laplacian matrix \f$L\f$ (including the diagonal).
   * The error at iteration \f$k\f$ is given by
   *
   * \f[
   *    e_k = \| L x_k - b \|_2,
   * \f]
   *
   * and we terminate the algorithm when \f$e_k\f$ is less than the provided
   * error parameter.
   *
   * \note
   * The Gauss-Seidel method is guaranteed to converge if the Laplacian matrix is
   * Strictly Diagonally Dominant (SDD), and may also converge in other cases.
   *
   * @param g the graph representing the Laplacian matrix to be used
   * @param b the vector \f$b\f$
   * @param eps the error parameter \f$\epsilon\f$ controlling the permitted
   *            approximation error.
   * @param max_iterations (optional) the maximum number of iterations to perform.
   *                       If this parameter is omitted, STAG will automatically
   *                       set the maximum iterations.
   * @return the approximate solution \f$\hat{x}\f$ such that
   *         \f$\| L \hat{x} - b \|_2 \leq \epsilon\f$.
   * @throws stag::ConvergenceError if the algorithm does not converge
   */
  DenseVec solve_laplacian_gauss_seidel(Graph* g, DenseVec& b, double eps,
                                        stag_int max_iterations);

  /**
   * @overload
   */
  DenseVec solve_laplacian_gauss_seidel(Graph* g, DenseVec& b, double eps);


  /**
   * Solve a linear system \f$A x = b\f$ by the Gauss-Seidel method.
   *
   * For more information about the Gauss-Seidel method, see
   * stag::solve_laplacian_gauss_seidel.
   *
   * @param A the matrix \f$A\f$
   * @param b the vector \f$b\f$
   * @param eps the error parameter \f$\epsilon\f$ controlling the permitted
   *            approximation error
   * @param max_iterations the maximum number of iterations to perform
   * @return the approximate solution \f$\hat{x}\f$ such that
   *         \f$\| A \hat{x} - b \|_2 \leq \epsilon\f$.
   * @throws stag::ConvergenceError if the algorithm does not converge
   */
  DenseVec gauss_seidel_iteration(const SprsMat* A, DenseVec& b, double eps,
                                  stag_int max_iterations);

  /**
   * Solve a Laplacian system \f$L x = b\f$ by the exact conjugate gradient
   * method.
   *
   * This method is not efficient, and an implementation is provided for educational
   * and research purposes.
   *
   * In the conjugate gradient method, we first compute \f$n\f$ vectors
   * \f$p_1, \ldots, p_n\f$ such that
   * \f[
   *    p_i^T L p_j = 0
   * \f]
   * for all \f$i \neq j\f$.
   * Then, \f$p_1, \ldots, p_n\f$ form a basis of \f$\mathbb{R}^n\f$ and we know
   * that any solution \f$x^*\f$ can be written as
   * \f[
   *    x^* = \sum_{i=1}^n \alpha_i p_i
   * \f]
   * and
   * \f[
   *    L x^* = \sum_{i=1}^n \alpha_i L p_i = b.
   * \f]
   * Multiplying both sides by an arbitrary \f$p_k\f$ and rearranging gives
   * \f[
   *    \alpha_k = \frac{p_k^T b}{p_k^T L p_k},
   * \f]
   * from which we can compute \f$x^*\f$.
   *
   * @param g the graph representing the Laplacian matrix to be used
   * @param b the vector \f$b\f$
   * @return the solution \f$x\f$ such that \f$L x = b\f$
   */
  DenseVec solve_laplacian_exact_conjugate_gradient(Graph* g, DenseVec& b);

  //----------------------------------------------------------------------------
  // Custom Exceptions for errors during the solve
  //----------------------------------------------------------------------------
  /**
   * @brief Exception thrown when an iterative solver fails to converge.
   */
  class ConvergenceError : public std::runtime_error {
  public:
    /**
     * Exception class constructor.
     */
    ConvergenceError()
      : std::runtime_error("Iterative solver failed to converge.") {}
  };
}

#endif //STAG_TEST_SOLVE_H
