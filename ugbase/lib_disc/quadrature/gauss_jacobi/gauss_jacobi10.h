/*
 * gauss_jacobi.h
 *
 *  Created on: 15.03.2013
 *      Author: lisagrau, andreasvogel
 */

#include "../quadrature.h"

#ifndef __H__UG__LIB_DISC__QUADRATURE__GAUSS_JACOBI10__
#define __H__UG__LIB_DISC__QUADRATURE__GAUSS_JACOBI10__

namespace ug{

/**
 * This class provides GaussJacobi integrals up to order 70
 * with alpha = 1 and beta = 0. For further information see e.g.
 *
 * Rathod, Venkatesh, Gauss Legendre - Gauss Jacobi Quadrature Rules over
 * a Tetrahedral Region, Int. J. Math Analysis, Vol. 5, 2011 (4), 189-198
 *
 * J. Villadsen and M.L. Michelsen, Solution of differential equation models by
 * polynomial approximation, Prentice Hall Inc, Englewood Cliffs,
 * New Jersey 07632 (1978)
 */
class GaussJacobi10 : public QuadratureRule<1>
{
	public:
	///	constructor
		GaussJacobi10(size_t order);

	///	destructor
		~GaussJacobi10();
};

} // namespace ug

#endif /* __H__UG__LIB_DISC__QUADRATURE__GAUSS_JACOBI10__ */
