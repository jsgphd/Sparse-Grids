// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/datadriven/application/LearnerPiecewiseConstantSmoothedRegression.hpp>

#include <sgpp/base/exception/application_exception.hpp>
#include <sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp>
#include <sgpp/base/grid/generation/GridGenerator.hpp>
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/grid/storage/hashmap/HashGridStorage.hpp>
#include <sgpp/base/operation/BaseOpFactory.hpp>
#include <sgpp/base/operation/hash/OperationEval.hpp>
#include <sgpp/base/operation/hash/OperationFirstMoment.hpp>
#include <sgpp/base/operation/hash/OperationMultipleEval.hpp>
#include <sgpp/pde/operation/PdeOpFactory.hpp>
#include <sgpp/solver/sle/ConjugateGradients.hpp>
#include <sgpp/datadriven/algorithm/PiecewiseConstantSmoothedRegressionSystemMatrix.hpp>

#include <stddef.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

using SGPP::base::GridStorage;
using SGPP::base::GridGenerator;
using SGPP::base::DataVector;
using SGPP::base::OperationMatrix;
using SGPP::base::Grid;
using SGPP::base::SurplusRefinementFunctor;
using SGPP::base::GridIndex;
using SGPP::base::OperationEval;
using SGPP::base::application_exception;

using std::endl;
using std::cout;

namespace SGPP {
namespace datadriven {

LearnerPiecewiseConstantSmoothedRegression::LearnerPiecewiseConstantSmoothedRegression(
  SGPP::base::RegularGridConfiguration& gridConfig,
  SGPP::base::AdpativityConfiguration& adaptivityConfig,
  SGPP::solver::SLESolverConfiguration& solverConfig,
  SGPP::datadriven::RegularizationConfiguration& regularizationConfig,
  bool verbose) :
  gridConfig(gridConfig), adaptivityConfig(adaptivityConfig),
  solverConfig(solverConfig), regularizationConfig(
    regularizationConfig), verbose(verbose) {
}

// ---------------------------------------------------------------------------

void LearnerPiecewiseConstantSmoothedRegression::train(
  SGPP::datadriven::PiecewiseConstantRegression::Node& piecewiseRegressor,
  Grid& grid,
  DataVector& alpha, float_t lambda) {
  size_t dim = grid.getStorage()->dim();

  GridStorage* gridStorage = grid.getStorage();
  GridGenerator* gridGen = grid.createGridGenerator();
  DataVector rhs(grid.getStorage()->size());
  alpha.resize(grid.getStorage()->size());
  alpha.setAll(0.0);

  if (verbose) {
    cout << "# LearnerDensityRegression: grid points " << grid.getSize() << endl;
  }

  for (size_t ref = 0; ref <= adaptivityConfig.numRefinements_; ref++) {
    OperationMatrix* C = computeRegularizationMatrix(grid);

    SGPP::datadriven::PiecewiseConstantSmoothedRegressionSystemMatrix SMatrix(
      piecewiseRegressor, grid, *C, lambda);

    if (verbose) {
      cout << "# integrating rhs" << std::endl;
    }

    SMatrix.generateb(rhs);

    if (verbose) {
      cout << "# LearnerDensityRegression: Solving " << endl;
    }

    SGPP::solver::ConjugateGradients myCG(solverConfig.maxIterations_,
                                          solverConfig.eps_);
    myCG.solve(SMatrix, alpha, rhs, false, false, solverConfig.threshold_);

    if (ref < adaptivityConfig.numRefinements_) {
      if (verbose) {
        cout << "# LearnerDensityRegression: Refine grid ... " << std::endl;
      }

      // Weight surplus with function evaluation at grid points
      OperationEval* opEval = SGPP::op_factory::createOperationEval(grid);
      GridIndex* gp;
      DataVector p(dim);
      DataVector alphaWeight(alpha.getSize());

      for (size_t i = 0; i < gridStorage->size(); i++) {
        gp = gridStorage->get(i);
        gp->getCoords(p);
        alphaWeight[i] = alpha[i] * opEval->eval(alpha, p);
      }

      delete opEval;
      opEval = NULL;

      SurplusRefinementFunctor srf(&alphaWeight, adaptivityConfig.noPoints_,
                                   adaptivityConfig.threshold_);
      gridGen->refine(&srf);

      if (verbose) {
        cout << "# LearnerDensityRegression: ref " << ref << "/" <<
             adaptivityConfig.numRefinements_ - 1 << ": "
             << grid.getStorage()->size() << endl;
      }

      alpha.resize(grid.getStorage()->size());
      rhs.resize(grid.getStorage()->size());
      alpha.setAll(0.0);
      rhs.setAll(0.0);
    }

    delete C;
  }

  return;
}

OperationMatrix*
LearnerPiecewiseConstantSmoothedRegression::computeRegularizationMatrix(
  SGPP::base::Grid& grid) {
  OperationMatrix* C = NULL;

  if (regularizationConfig.regType_ ==
      SGPP::datadriven::RegularizationType::Identity) {
    C = SGPP::op_factory::createOperationIdentity(grid);
  } else if (regularizationConfig.regType_ ==
             SGPP::datadriven::RegularizationType::Laplace) {
    C = SGPP::op_factory::createOperationLaplace(grid);
  } else {
    throw application_exception("LearnerDensityRegression::train : unknown regularization type");
  }

  return C;
}

}  // namespace datadriven
}  // namespace SGPP
