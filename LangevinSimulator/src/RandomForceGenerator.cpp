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

#include <iostream>
#include <sys/time.h>
#include <cmath>
#include <cstring>
#include <iostream>

#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_randist.h>

using namespace std;
using namespace TensorUtils;

RandomForceGenerator::RandomForceGenerator()
{
    // set up GSL random number generator
    gsl_rng_env_setup();
    rng_T = gsl_rng_default;
    rng_r = gsl_rng_alloc (rng_T);

    // generate random seed
    struct timeval tv;
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    gsl_rng_set(rng_r, mySeed);
}

void RandomForceGenerator::init_cov(tensor<double,2> &ff_average, tensor<double,4> &ff_cov, string out_folder)
{
    // store average
    this->ff_average = ff_average;

    size_t num_ts = ff_cov.shape[0];
    size_t num_obs = ff_cov.shape[1];
    size_t n_max = num_ts*num_obs;

    // allocate
    ff_decomp = gsl_matrix_alloc(n_max,n_max);
    buffer = gsl_vector_alloc(n_max);
    buffer2 = gsl_vector_alloc(n_max);
    rand_mult_gaussian.alloc({num_ts,num_obs});

    // initialize
    cout << "Calculate rotation matrix using spectral decomposition." << endl;
    set_decomp(ff_cov,ff_decomp);

    ff_cov << *ff_decomp->data;
    cout << "Write rotation matrix: " << out_folder+"/"+"ff_decomp.f64" << endl;
    ff_cov.write("ff_decomp.f64",out_folder);
}

void RandomForceGenerator::init_decomp(tensor<double,2> &ff_average, tensor<double,4> &ff_decomp)
{
    // store average
    this->ff_average = ff_average;

    size_t num_ts = ff_decomp.shape[0];
    size_t num_obs = ff_decomp.shape[1];
    size_t n_max = num_ts*num_obs;

    // allocate
    this->ff_decomp = gsl_matrix_alloc(n_max,n_max);
    buffer = gsl_vector_alloc(n_max);
    buffer2 = gsl_vector_alloc(n_max);
    rand_mult_gaussian.alloc({num_ts,num_obs});

    ff_decomp >> *this->ff_decomp->data;
}

RandomForceGenerator::~RandomForceGenerator()
{
    // free GSL random number generator
    gsl_rng_free (rng_r);
    gsl_matrix_free(ff_decomp);
    gsl_vector_free(buffer);
    gsl_vector_free(buffer2);
}

void RandomForceGenerator::set_decomp(tensor<double,4> &source, gsl_matrix *dest)
{
    source >> *dest->data;
    size_t n_max = source.shape[0]*source.shape[1];

    gsl_vector *eval = gsl_vector_alloc (n_max);
    gsl_matrix *evec = gsl_matrix_alloc (n_max, n_max);

    gsl_eigen_symmv_workspace * w =
    gsl_eigen_symmv_alloc (n_max);

    gsl_eigen_symmv (dest, eval, evec, w);

    gsl_eigen_symmv_free (w);

    gsl_eigen_symmv_sort (eval, evec, GSL_EIGEN_SORT_ABS_ASC);

    for(size_t i=0; i<n_max; i++)
    {
        for(size_t j=0; j<n_max; j++)
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
    gsl_blas_dgemv(CblasNoTrans,1.0,ff_decomp,buffer,0.0,buffer2);

    // store data and return
    rand_mult_gaussian << *buffer2->data;
    rand_mult_gaussian += ff_average;
    return rand_mult_gaussian;
}
