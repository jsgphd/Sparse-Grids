// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef PHIPHIUPMODLINEAR_HPP
#define PHIPHIUPMODLINEAR_HPP

#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>

#include <sgpp/globaldef.hpp>

namespace SGPP {
namespace pde {

/**
 * Implementation of sweep operator (): 1D Up for
 * Bilinearform \f$\int_{x} \phi(x) \phi(x) dx\f$
 * on mod-linear grids
 */
class PhiPhiUpModLinear {
 protected:
  typedef SGPP::base::GridStorage::grid_iterator grid_iterator;
  /// Pointer to the SGPP::base::GridStorage object
  SGPP::base::GridStorage* storage;

 public:
  /**
   * Constructor
   *
   * @param storage the grid's SGPP::base::GridStorage object
   */
  explicit PhiPhiUpModLinear(SGPP::base::GridStorage* storage);

  /**
   * Destructor
   */
  ~PhiPhiUpModLinear();

  /**
   * This operations performs the calculation of up in the direction of dimension <i>dim</i>
   *
   * @param source SGPP::base::DataVector that contains the gridpoint's coefficients (values from
   * the vector of the laplace operation)
   * @param result SGPP::base::DataVector that contains the result of the up operation
   * @param index a iterator object of the grid
   * @param dim current fixed dimension of the 'execution direction'
   */
  void operator()(SGPP::base::DataVector& source, SGPP::base::DataVector& result,
                  grid_iterator& index, size_t dim);

 protected:
  /**
   * recursive function for the calculation of Up
   *
   * @param source SGPP::base::DataVector that contains the coefficients of the ansatzfunction
   * @param result SGPP::base::DataVector in which the result of the operation is stored
   * @param index reference to a griditerator object that is used navigate through the grid
   * @param dim the dimension in which the operation is executed
   * @param fl function value on the left boundary
   * @param fr function value on the right boundary
   */
  void rec(SGPP::base::DataVector& source, SGPP::base::DataVector& result, grid_iterator& index,
           size_t dim, float_t& fl, float_t& fr);
};
}  // namespace pde
}  // namespace SGPP

#endif /* PHIPHIUPMODLINEAR_HPP */
