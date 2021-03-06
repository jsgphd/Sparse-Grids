// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SLESOLVER_HPP
#define SLESOLVER_HPP

#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/operation/hash/OperationMatrix.hpp>

#include <sgpp/solver/SGSolver.hpp>

#ifndef DEFAULT_RES_THRESHOLD
#define DEFAULT_RES_THRESHOLD -1.0
#endif

#include <sgpp/globaldef.hpp>

namespace SGPP {
namespace solver {

class SLESolver : public SGSolver {
 public:
  /**
   * Std-Constructor
   *
   * @param imax number of maximum executed iterations
   * @param epsilon the final error in the iterative solver
   */
  SLESolver(size_t imax, float_t epsilon) : SGSolver(imax, epsilon) {}

  /**
   * Std-Destructor
   */
  virtual ~SLESolver() {}

  /**
   * Pure virtual Function that defines a solve method for an iterative solver
   *
   * @param SystemMatrix reference to an SGPP::base::OperationMatrix Object that implements the
   * matrix vector multiplication
   * @param alpha the sparse grid's coefficients which have to be determined
   * @param b the right hand side of the system of linear equations
   * @param reuse identifies if the alphas, stored in alpha at calling time, should be reused
   * @param verbose prints information during execution of the solver
   * @param max_threshold additional abort criteria for solver, default value is 10^-9!
   */
  virtual void solve(SGPP::base::OperationMatrix& SystemMatrix, SGPP::base::DataVector& alpha,
                     SGPP::base::DataVector& b, bool reuse = false, bool verbose = false,
                     float_t max_threshold = DEFAULT_RES_THRESHOLD) = 0;
};

}  // namespace solver
}  // namespace SGPP

#endif /* SLESOLVER_HPP */
