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

#include "RandomForceGenerator.hpp"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_linalg.h>
#include <iostream>
#include <sys/time.h>
#include <cmath>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <cstring>
#include <iostream>

using namespace std;
using namespace TensorUtils;

RandomForceGenerator::RandomForceGenerator(tensor<double,1> &average, tensor<double,2> &cov, size_t num_obs)
{
    // store average
    this->average = average;

    // set up GSL random number generator
    gsl_rng_env_setup();
    rng_T = gsl_rng_default;
    rng_r = gsl_rng_alloc (rng_T);

    // generate random seed
    struct timeval tv;
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    gsl_rng_set(rng_r, mySeed);

    size_t n_max = cov.shape[0];
    size_t num_ts = n_max/num_obs;

    // allocate
    f_t_s_decomp = gsl_matrix_alloc(n_max,n_max);
    buffer = gsl_vector_alloc(n_max);
    buffer2 = gsl_vector_alloc(n_max);
    rand_mult_gaussian.alloc({num_ts,num_obs},1.0);

    // initialize
    set_decomp(cov,f_t_s_decomp);
}

RandomForceGenerator::~RandomForceGenerator()
{
    // free GSL random number generator
    gsl_rng_free (rng_r);
    gsl_matrix_free(f_t_s_decomp);
    gsl_vector_free(buffer);
    gsl_vector_free(buffer2);
}

void RandomForceGenerator::set_decomp(tensor<double,2> &source, gsl_matrix *dest)
{
    source >> *dest->data;

    gsl_vector *eval = gsl_vector_alloc (source.shape[0]);
    gsl_matrix *evec = gsl_matrix_alloc (source.shape[0], source.shape[0]);

    gsl_eigen_symmv_workspace * w =
    gsl_eigen_symmv_alloc (source.shape[0]);

    gsl_eigen_symmv (dest, eval, evec, w);

    gsl_eigen_symmv_free (w);

    gsl_eigen_symmv_sort (eval, evec, GSL_EIGEN_SORT_ABS_ASC);

    for(size_t i=0; i<source.shape[0]; i++)
    {
        for(size_t j=0; j<source.shape[0]; j++)
        {
            double dummy = sqrt(gsl_vector_get(eval,j));
            if(!isfinite(dummy))
            {
                //cout << "Warning: negative eigenvalue in spectral decomp: " << gsl_vector_get(eval,j) << endl;
                dummy = 0.0;
            }
            gsl_matrix_set(dest,i,j,gsl_matrix_get(evec,i,j)*dummy);
        }
    }

    gsl_vector_free (eval);
    gsl_matrix_free (evec);
}

tensor<double,2> RandomForceGenerator::pull_multivariate_gaussian()
{
    // draw independent standard normal distributed variables
    for(size_t n=0; n<buffer->size; n++)
    {
        gsl_vector_set(buffer,n,gsl_ran_gaussian(rng_r,1.0));
    }

    // compute multi-dimensional Gaussian variables
    gsl_blas_dgemv(CblasNoTrans,1.0,f_t_s_decomp,buffer,0.0,buffer2);

    // store data and return
    rand_mult_gaussian << *buffer2->data;
    rand_mult_gaussian += average;
    return rand_mult_gaussian;
}
