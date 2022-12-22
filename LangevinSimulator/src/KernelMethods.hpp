/**
LangevinSimulator Version 1.0

Copyright 2020-2022 Christoph Widder and Fabian Glatzel

Christoph Widder <christoph.widder[at]merkur.uni-freiburg.de>
Fabian Glatzel <fabian.glatzel[at]physik.uni-freiburg.de>

This file is part of LangevinSimulator.

LangevinSimulator is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

LangevinSimulator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LangevinSimulator.
If not, see <https://www.gnu.org/licenses/>.
**/

#ifndef KERNEL_KERNELMETHODS_H_
#define KERNEL_KERNELMETHODS_H_

#include <gsl/gsl_matrix.h>
#include <vector>
#include <string>

#include "TensorUtils.hpp"

/*!
    \brief This namespace provides functions to compute the memory kernel, drift and fluctuating forces.
*/
namespace KernelMethods {

    TensorUtils::tensor<double,4> calcLowerBlockTriangularInverse(TensorUtils::tensor<double,4> &src);

    //! \private
	void calcDiagInverts(
		gsl_matrix const* corr,
		gsl_matrix **diag_inverts);

    //! \private
	void calcDerivative(
		gsl_matrix const* mat,
		size_t const num_ts,
		size_t const num_obs,
		gsl_matrix* deriv,
		double const dt,
		bool const firstTime=true);

    //! \private
	void calcS0(
		gsl_matrix const* corr,
		gsl_matrix* temp,
		gsl_matrix** diag_inverts,
		double const dt,
		gsl_matrix* S0);

    //! \private
	void calcj(
		gsl_matrix const* corr,
		gsl_matrix* temp,
		gsl_matrix** diag_inverts,
		double const dt,
		gsl_matrix* j);

    //! \private
	void splitIntoTriangular(
		gsl_matrix const* mat,
		int const num_ts,
		int const num_obs,
		gsl_matrix* lower,
		gsl_matrix* upper,
		bool const flipLower=false);

    //! \private
	void calcJ(
		gsl_matrix* S0,
		gsl_matrix* j,
		gsl_matrix* temp1,
		gsl_matrix* temp2,
		gsl_matrix* J,
		int const num_ts,
		int const num_obs,
		double const dt);

    /*!
        \brief Computes the memory kernel for a given correlation function.
    */
	void getMemoryKernel(
		gsl_matrix* kernel,
		gsl_matrix* corr,
		size_t const num_ts,
		size_t const num_obs,
		double const dt);
    TensorUtils::tensor<double,3> getMemoryKernel(TensorUtils::tensor<double,3> &correlation, double dt);

    /*!
        \brief Computes the derivative of all trajectories using the symmetric difference quotient.

        At the boundary the one-sided difference quotient is used.
    */
    TensorUtils::tensor<double,3> diffTrajectories(TensorUtils::tensor<double,3> &trajectories, double dt, bool darboux_sum);

    /*!
        \brief Block-wise matrix inversion.

        \return Returns a tensor defined by `return_val(n,:,:)=mat(n,:,:)^{-1}`.
    */
    TensorUtils::tensor<double,3> matInverse(TensorUtils::tensor<double,3> &mat);

    TensorUtils::tensor<double,3> diffFront(TensorUtils::tensor<double,3> &correlation, double dt);

    TensorUtils::tensor<double,3> getDrift(TensorUtils::tensor<double,4> &correlation, double dt);
    TensorUtils::tensor<double,2> getDrift(TensorUtils::tensor<double,3> &correlation, double dt);


    /*!
        \brief Computes the drift term and the fluctuating forces.

        The drift term is written to 'out_folder/drift.f64' as binary. If `txt_out`, the drift term is written as text file as well.

        \return Returns the fluctuating forces for each trajectory.
    */
    TensorUtils::tensor<double,3> getFluctuatingForce(
        TensorUtils::tensor<double,4> &kernel,
        TensorUtils::tensor<double,3> &drift,
        TensorUtils::tensor<double,3> &trajectories,
        TensorUtils::tensor<double,1> &times,
        bool darboux_sum);

    /*!
        \brief Computes the covariance matrix of the fluctuating forces as required from
        \ref RandomForceGenerator.

        The covariance matrix is written to `out_folder/cov.64`.
    */
    void writeCovarianceMatrix(TensorUtils::tensor<double,3> &ff, std::string out_folder);

    void writeExtendedCovarianceMatrix(TensorUtils::tensor<double,3> &traj, TensorUtils::tensor<double,3> &ff, std::string out_folder);
}

#endif  // KERNEL_KERNELMETHODS_H_
