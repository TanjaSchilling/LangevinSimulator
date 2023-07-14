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
#include <filesystem>

#include "TensorUtils.hpp"

/*!
    \brief This class offers routines to generate multivariate normal distributed fluctuating forces.
*/
class RandomForceGenerator
{
    public:
        /*!
            \brief Returns an uninitialized instantiation, but sets up the random number generator.
        */
        RandomForceGenerator();
        virtual ~RandomForceGenerator();

        /*!
            \brief Computes and writes the rotation matrix for a given covariance matrix. On exit, the RandomForceGenerator is initialized.
        */
        void init_cov(TensorUtils::tensor<double,2> &ff_average, TensorUtils::tensor<double,4> &ff_cov, std::filesystem::path out_path);
        void init_cov(TensorUtils::tensor<double,2> &ff_average, TensorUtils::tensor<double,3> &ff_cov, std::filesystem::path out_path);

        /*!
            \brief Initializes the RandomForceGenerator with a previously computed rotation matrix.
        */
        void init_decomp(TensorUtils::tensor<double,2> &ff_average, TensorUtils::tensor<double,4> &ff_decomp);

        /*!
            \brief Draw multivariate normal distributed fluctuating forces.

            \return Returns the fluctuating forces `rand_ff = Mz`.
            The rotation matrix `M` is given by `M=UD^{1/2}`, where `C=UDU^{-1}` is the spectral decomposition of the covariance matrix
            and `z` contains independent, standard normal distributed random numbers.
        */
        TensorUtils::tensor<double,2> pull_multivariate_gaussian();

        //! \private
        TensorUtils::tensor<double,2> pull_stationary_multivariate_gaussian();

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

        //! \private
        std::vector<gsl_matrix_complex*> stationary_decomp;
        size_t num_obs;
        size_t num_ts;
        size_t num_pad;
        double * lookup_table = nullptr;
};

#endif // RANDOMFORCEGENERATOR_HPP
