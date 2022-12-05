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

#ifndef RANDOMFORCEGENERATOR_HPP
#define RANDOMFORCEGENERATOR_HPP

#include <vector>
#include <string>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>

#include "TensorUtils.hpp"

/*!
    \brief This class offers routines to generate multivariate normal distributed fluctuating forces.
*/
class RandomForceGenerator
{
    public:
        /*!
            \brief Constructor expects the mean and covariance matrix of the fluctuating forces.

            After the RandomForceGenerator has been constructed, the fluctuating forces can be drawn by calling \ref pull_multivariate_gaussian().

            \param average Mean value of the fluctuating forces as a one-dimensional tensor: `ff_average[num_obs*t+o]=<ff(t,o)>`.
            \param cov Covariance matrix of the fluctuating forces `ff_cov(num_obs*t1+o1,num_obs*t2+o2)=<ff(t1,o1)*ff(t2,o2)>-<ff(t1,o1)>*<ff(t2,o2)>`.
            \param num_obs Number of observables.
        */
        RandomForceGenerator();
        virtual ~RandomForceGenerator();

        void init_cov(TensorUtils::tensor<double,2> &ff_average, TensorUtils::tensor<double,4> &ff_cov, std::string out_folder);
        void init_decomp(TensorUtils::tensor<double,2> &ff_average, TensorUtils::tensor<double,4> &ff_decomp);

        /*!
            \brief Draw multivariate normal distributed fluctuating forces.

            \return Returns the fluctuating forces `rand_ff = Mz`.
            The matrix `M` is given by `M=UD^{1/2}`, where `C=UDU^{-1}` is the spectral decomposition of the covariance matrix
            and `z` contains independent, standard normal distributed numbers.

        */
        TensorUtils::tensor<double,2> pull_multivariate_gaussian();

    protected:

        /*!
            \brief Computes `M=UD^{1/2}`, where `C=UDU^{-1}` is the spectral decomposition of the covariance matrix of the fluctuating forces.

            This function is called within the constructor and stores the matrix `M` in an internal buffer to be used by \ref pull_multivariate_gaussian().
        */
        void set_decomp(TensorUtils::tensor<double,4> &source, gsl_matrix *dest);

        //! \private
        TensorUtils::tensor<double,2> rand_mult_gaussian;

        //! \private
        gsl_matrix *ff_decomp;
        //! \private
        gsl_vector *buffer;
        //! \private
        gsl_vector *buffer2;
        //! \private
        TensorUtils::tensor<double,2> ff_average;

        //! \private
        const gsl_rng_type * rng_T;
        //! \private
        gsl_rng * rng_r;
};

#endif // RANDOMFORCEGENERATOR_HPP
