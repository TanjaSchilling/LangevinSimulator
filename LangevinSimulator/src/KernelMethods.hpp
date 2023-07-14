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

#include "TensorUtils.hpp"
#include "RandomForceGenerator.hpp"

#include <gsl/gsl_matrix.h>
#include <vector>
#include <string>
#include <filesystem>

/*!
    \brief This namespace provides functions to compute the memory kernel, drift and fluctuating forces.
*/
namespace KernelMethods {

    /*!
        \brief Subtracts and returns the average of the trajectories.
    */
    TensorUtils::tensor<double,1> shiftTrajectories(TensorUtils::tensor<double,3> &traj);

    /*!
        \brief Fast convolution with a mollifier of sample-width `2*mollifier_width-1`.
    */
    void mollifyTrajectories(TensorUtils::tensor<double,1> &times, TensorUtils::tensor<double,3> &traj, size_t mollifier_width);

    /*!
        \brief Returns the cross-correlation matrix of the observable `C(t,i,s,j)=E[traj(n,t,i)*traj(n,s,j)]` for all times `t` and `s`.
    */
    TensorUtils::tensor<double,4> getCorrelationFunction(TensorUtils::tensor<double,3> &traj, bool unbiased=false);

    /*!
        \brief Returns the cross-correlation matrix of a stationary process
        `C(num_ts-1+t-s,i,j)=E[traj(n,t,i)*traj(n,s,j)]` for all time differences `t-s=-(N-1),...,+(N-1)`.
    */
    TensorUtils::tensor<double,3> getStationaryCorrelation(TensorUtils::tensor<double,3> &traj, bool unbiased=false);

    /*!
        \brief Subtracts and returns the average trajectory.
    */
    TensorUtils::tensor<double,2> subAverage(TensorUtils::tensor<double,3> &traj);

    /*!
        \brief Returns the inverse of a lower block triangular matrix.
    */
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
        \brief Computes the non-stationary memory kernel for a given correlation function.
    */
	void getMemoryKernel(
		gsl_matrix* kernel,
		gsl_matrix* corr,
		size_t const num_ts,
		size_t const num_obs,
		double const dt);
    /*!
        \brief Computes the stationary memory kernel for a given correlation function.
    */
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

    /*!
        \brief Differentiate with respect to the first index using the symmetric difference quotient.
        For the first and last index, the one-sided difference quotient is used.
    */
    TensorUtils::tensor<double,3> diffFront(TensorUtils::tensor<double,3> &correlation, double dt);

    /*!
        \brief Returns the drift term for a given non-stationary correlation function.
    */
    TensorUtils::tensor<double,3> getDrift(TensorUtils::tensor<double,4> &correlation, double dt);

    /*!
        \brief Returns the drift term for a given stationary correlation function.
    */
    TensorUtils::tensor<double,2> getDrift(TensorUtils::tensor<double,3> &correlation, double dt);


    /*!
        \brief Returns the fluctuating forces for each trajectory for the non-stationary case.
    */
    TensorUtils::tensor<double,3> getFluctuatingForce(
        TensorUtils::tensor<double,4> &kernel,
        TensorUtils::tensor<double,3> &drift,
        TensorUtils::tensor<double,3> &trajectories,
        TensorUtils::tensor<double,1> &times,
        bool darboux_sum);

    /*!
        \brief Returns the fluctuating forces for each trajectory for the stationary case.
    */
    TensorUtils::tensor<double,3> getFluctuatingForce(
        TensorUtils::tensor<double,3> &kernel,
        TensorUtils::tensor<double,2> &drift,
        TensorUtils::tensor<double,3> &trajectories,
        TensorUtils::tensor<double,1> &times,
        bool darboux_sum);

    /*!
        \brief Computes the average and covariance matrix of the fluctuating forces as required from
        \ref RandomForceGenerator.

        The average fluctuating forces are written to `out_path/ff_average.f64`.
        The covariance matrix is written to `out_path/ff_cov.f64` or `out_path/ff_cov_stationary.f64` in the stationary case.
    */
    void writeCovarianceMatrix(TensorUtils::tensor<double,3> &ff, std::filesystem::path out_path, bool stationary=false);

    /*!
        \brief Computes the average and covariance matrix of the initial values and fluctuating forces as required from
        \ref RandomForceGenerator when drawing the initial values and fluctuating forces from a joint distribution.

        The average initial values and fluctuating forces are written to `out_path/ff_average.f64`.
        The covariance matrix of initial values and fluctuating forces is written to `out_path/ff_cov.f64`.
        In the stationary case, the covariance matrix of the fluctuating forces is written to `out_path/ff_cov_stationary.f64`
        and the covariance between the initial values and fluctuating forces is written to `out_path/ff_cov_extended.f64`.
    */
    void writeExtendedCovarianceMatrix(TensorUtils::tensor<double,3> &traj, TensorUtils::tensor<double,3> &ff, std::filesystem::path out_path, bool stationary=false);

    /*!
        \brief Draws the fluctuating forces and simulates new trajectories for the non-stationary case.
    */
    TensorUtils::tensor<double,3> simulateTrajectories(
        TensorUtils::tensor<double,3> &traj,
        TensorUtils::tensor<double,3> &drift,
        TensorUtils::tensor<double,4> &kernel,
        TensorUtils::tensor<double,1> &mean_initial_value,
        RandomForceGenerator &rfg,
        double dt,
        bool shift,
        bool gaussian_init_val,
        bool darboux_sum,
        size_t num_sim,
        std::filesystem::path out_path);

    /*!
        \brief Draws the fluctuating forces and simulates new trajectories for the stationary case.
    */
    TensorUtils::tensor<double,3> simulateTrajectories(
        TensorUtils::tensor<double,3> &traj,
        TensorUtils::tensor<double,2> &drift,
        TensorUtils::tensor<double,3> &kernel,
        TensorUtils::tensor<double,1> &mean_initial_value,
        RandomForceGenerator &rfg,
        double dt,
        bool shift,
        bool gaussian_init_val,
        bool darboux_sum,
        size_t num_sim,
        std::filesystem::path out_path);
}

#endif  // KERNEL_KERNELMETHODS_H_
