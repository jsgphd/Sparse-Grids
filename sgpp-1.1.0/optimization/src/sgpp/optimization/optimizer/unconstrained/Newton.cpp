// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/globaldef.hpp>

#include <sgpp/optimization/optimizer/unconstrained/Newton.hpp>
#include <sgpp/optimization/optimizer/unconstrained/LineSearchArmijo.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>
#include <sgpp/optimization/sle/system/FullSLE.hpp>
#include <sgpp/optimization/tools/Printer.hpp>

#include <algorithm>
#include <numeric>

namespace SGPP {
namespace optimization {
namespace optimizer {

Newton::Newton(ScalarFunction& f, ScalarFunctionHessian& fHessian, size_t max_it_count,
               float_t beta, float_t gamma, float_t tolerance, float_t epsilon, float_t alpha1,
               float_t alpha2, float_t p)
    : UnconstrainedOptimizer(f, max_it_count),
      fHessian(fHessian),
      beta(beta),
      gamma(gamma),
      tol(tolerance),
      eps(epsilon),
      alpha1(alpha1),
      alpha2(alpha2),
      p(p),
      defaultSleSolver(sle_solver::GaussianElimination()),
      sleSolver(defaultSleSolver) {}

Newton::Newton(ScalarFunction& f, ScalarFunctionHessian& fHessian, size_t max_it_count,
               float_t beta, float_t gamma, float_t tolerance, float_t epsilon, float_t alpha1,
               float_t alpha2, float_t p, const sle_solver::SLESolver& sleSolver)
    : UnconstrainedOptimizer(f, max_it_count),
      fHessian(fHessian),
      beta(beta),
      gamma(gamma),
      tol(tolerance),
      eps(epsilon),
      alpha1(alpha1),
      alpha2(alpha2),
      p(p),
      defaultSleSolver(sle_solver::GaussianElimination()),
      sleSolver(sleSolver) {}

Newton::~Newton() {}

void Newton::optimize() {
  Printer::getInstance().printStatusBegin("Optimizing (Newton)...");

  const size_t d = f.getNumberOfParameters();

  xOpt.resize(0);
  fOpt = NAN;
  xHist.resize(0, d);
  fHist.resize(0);

  base::DataVector x(x0);
  float_t fx = NAN;

  base::DataVector gradFx(d);
  base::DataMatrix hessianFx(d, d);
  base::DataVector dk(d);
  base::DataVector s(d);
  base::DataVector y(d);

  FullSLE system(hessianFx);
  size_t k = 0;
  const bool statusPrintingEnabled = Printer::getInstance().isStatusPrintingEnabled();

  while (k < N) {
    // calculate gradient, Hessian and gradient norm
    fx = fHessian.eval(x, gradFx, hessianFx);
    k++;

    if (k == 1) {
      xHist.appendRow(x);
      fHist.append(fx);
    }

    const float_t gradFxNorm = gradFx.l2Norm();

    // exit if norm small enough
    if (gradFxNorm < tol) {
      break;
    }

    // RHS of linear system to be solved
    for (size_t t = 0; t < d; t++) {
      s[t] = -gradFx[t];
    }

    // solve linear system with Hessian as system matrix
    if (statusPrintingEnabled) {
      Printer::getInstance().disableStatusPrinting();
    }

    bool lsSolved = sleSolver.solve(system, s, dk);

    if (statusPrintingEnabled) {
      Printer::getInstance().enableStatusPrinting();
    }

    // norm of solution
    const float_t dkNorm = dk.l2Norm();

    // acceptance criterion
    if (lsSolved &&
        (s.dotProduct(dk) >= std::min(alpha1, alpha2 * std::pow(dkNorm, p)) * dkNorm * dkNorm)) {
      // normalized solution as new search direction
      for (size_t t = 0; t < d; t++) {
        s[t] = dk[t] / dkNorm;
      }
    } else {
      // restart method
      // (negated normalized gradient as new search direction)
      for (size_t t = 0; t < d; t++) {
        s[t] = s[t] / gradFxNorm;
      }
    }

    // status printing
    Printer::getInstance().printStatusUpdate(std::to_string(k) + " evaluations, x = " +
                                             x.toString() + ", f(x) = " + std::to_string(fx));

    // line search
    if (!lineSearchArmijo(f, beta, gamma, tol, eps, x, fx, gradFx, s, y, k)) {
      // line search failed ==> exit
      // (either a "real" error occured or the improvement
      // achieved is too small)
      break;
    }

    x = y;
    xHist.appendRow(x);
    fHist.append(fx);
  }

  xOpt.resize(d);
  xOpt = x;
  fOpt = fx;
  Printer::getInstance().printStatusEnd();
}

ScalarFunctionHessian& Newton::getObjectiveHessian() const { return fHessian; }

float_t Newton::getBeta() const { return beta; }

void Newton::setBeta(float_t beta) { this->beta = beta; }

float_t Newton::getGamma() const { return gamma; }

void Newton::setGamma(float_t gamma) { this->gamma = gamma; }

float_t Newton::getTolerance() const { return tol; }

void Newton::setTolerance(float_t tolerance) { tol = tolerance; }

float_t Newton::getEpsilon() const { return eps; }

void Newton::setEpsilon(float_t epsilon) { eps = epsilon; }

float_t Newton::getAlpha1() const { return alpha1; }

void Newton::setAlpha1(float_t alpha1) { this->alpha1 = alpha1; }

float_t Newton::getAlpha2() const { return alpha2; }

void Newton::setAlpha2(float_t alpha2) { this->alpha2 = alpha2; }

float_t Newton::getP() const { return p; }

void Newton::setP(float_t p) { this->p = p; }

void Newton::clone(std::unique_ptr<UnconstrainedOptimizer>& clone) const {
  clone = std::unique_ptr<UnconstrainedOptimizer>(new Newton(*this));
}
}  // namespace optimizer
}  // namespace optimization
}  // namespace SGPP
