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
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

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

RandomForceGenerator::~RandomForceGenerator()
{
    // free GSL random number generator
    gsl_rng_free (rng_r);
    gsl_matrix_free(ff_decomp);
    gsl_vector_free(buffer);
    gsl_vector_free(buffer2);
    for(auto it=stationary_decomp.begin(); it!=stationary_decomp.end(); it++)
    {
        gsl_matrix_complex_free(*it);
    }
}

void RandomForceGenerator::init_cov(tensor<double,2> &ff_average, tensor<double,4> &ff_cov, filesystem::path out_path)
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
    cout << "Write rotation matrix: " << out_path/"ff_decomp.f64" << endl;
    ff_cov.write("ff_decomp.f64",out_path);
}

void RandomForceGenerator::init_cov(tensor<double,2> &ff_average, tensor<double,3> &ff_cov, filesystem::path out_path)
{
    this->ff_average = ff_average;
    num_ts = ff_average.shape[0];
    num_obs = ff_average.shape[1];
    num_pad = 2*num_ts-1;
    tensor<double, 3> corr_pad({num_obs,num_obs,num_pad},0.0);
    for(size_t o2=0;o2<num_obs;o2++)
    {
        for(size_t o1=0;o1<num_obs;o1++)
        {
            for(int t=0;t<(int)num_ts;t++)
            {
                corr_pad(o1,o2,t) = ff_cov(num_ts-1+t,o1,o2);
                if(t!=0)
                {
                    corr_pad(o1,o2,num_pad-t) = ff_cov(num_ts-1-t,o1,o2);
                }
            }
        }
    }
    gsl_fft_real_wavetable * real = gsl_fft_real_wavetable_alloc (num_pad);
    gsl_fft_real_workspace * work = gsl_fft_real_workspace_alloc (num_pad);
    for(size_t o2=0;o2<num_obs;o2++)
    {
        for(size_t o1=0;o1<num_obs;o1++)
        {
            gsl_fft_real_transform (&corr_pad(o1,o2), 1, num_pad, real, work);
        }
    }
    gsl_fft_real_workspace_free(work);
    gsl_fft_real_wavetable_free (real);
    stationary_decomp.resize(num_ts);
    for(auto it=stationary_decomp.begin(); it!=stationary_decomp.end(); it++)
    {
        *it = gsl_matrix_complex_alloc(num_obs,num_obs);
    }
    gsl_matrix_complex *cmat = gsl_matrix_complex_alloc(num_obs, num_obs);
    gsl_vector *eval = gsl_vector_alloc (num_obs);
    gsl_eigen_hermv_workspace * w = gsl_eigen_hermv_alloc(num_obs);
    for(size_t o2=0;o2<num_obs;o2++)
    {
        for(size_t o1=0;o1<num_obs;o1++)
        {
            gsl_matrix_complex_set(cmat, o1, o2, {corr_pad(o1,o2,0), 0.0});
        }
    }
    gsl_eigen_hermv(cmat, eval, stationary_decomp[0], w);
    for(size_t o2=0;o2<num_obs;o2++)
    {
        double tmp = gsl_vector_get(eval,o2);
        if(tmp<0)
        {
            tmp=0;
        }
        else
        {
            tmp = sqrt(tmp);
        }
        for(size_t o1=0;o1<num_obs;o1++)
        {
            gsl_complex cvalue = gsl_matrix_complex_get(stationary_decomp[0], o1, o2);
            gsl_matrix_complex_set(stationary_decomp[0], o1, o2, gsl_complex_mul(cvalue,{tmp, 0.0}));
        }
    }
    for(size_t k=1; k<num_ts; k++)
    {
        for(size_t o2=0;o2<num_obs;o2++)
        {
            for(size_t o1=0;o1<num_obs;o1++)
            {
                gsl_matrix_complex_set(cmat,o1,o2,{corr_pad(o1,o2,2*k-1),corr_pad(o1,o2,2*k)});
            }
        }
        gsl_eigen_hermv(cmat, eval, stationary_decomp[k], w);
        for(size_t o2=0;o2<num_obs;o2++)
        {
            double tmp = gsl_vector_get(eval,o2);
            if(tmp<0)
            {
                tmp=0;
            }
            else
            {
                tmp = sqrt(tmp);
            }
            for(size_t o1=0;o1<num_obs;o1++)
            {
                gsl_complex cvalue = gsl_matrix_complex_get(stationary_decomp[k], o1, o2);
                gsl_matrix_complex_set(stationary_decomp[k], o1, o2, gsl_complex_mul(cvalue,{tmp, 0.0}));
            }
        }
    }
    gsl_eigen_hermv_free(w);
    gsl_vector_free(eval);
    gsl_matrix_complex_free(cmat);
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

tensor<double,2> RandomForceGenerator::pull_stationary_multivariate_gaussian()
{
    gsl_vector_complex *buffer = gsl_vector_complex_alloc(num_obs);
    gsl_vector_complex *buffer2 = gsl_vector_complex_alloc(num_obs);
    tensor<double,2> ff_coeff({num_obs, num_pad});
    for(size_t k=0; k<num_ts; k++)
    {
        for(size_t o=0; o<num_obs; o++)
        {
            gsl_vector_complex_set(buffer,o,{gsl_ran_gaussian(rng_r,1.0/sqrt(2)), gsl_ran_gaussian(rng_r,1.0)/sqrt(2)});
        }
        gsl_blas_zgemv(CblasNoTrans,{1.0, 0.0},stationary_decomp[k],buffer,{0.0, 0.0},buffer2);
        for(size_t o=0; o<num_obs; o++)
        {
            gsl_complex cvalue = gsl_vector_complex_get(buffer2,o);
            if(k==0)
            {
                ff_coeff(o,0) = cvalue.dat[0]*sqrt(2);
            }
            else
            {
                ff_coeff(o,2*k-1) = cvalue.dat[0];
                ff_coeff(o,2*k) = cvalue.dat[1];
            }
        }
    }
    gsl_vector_complex_free(buffer);
    gsl_vector_complex_free(buffer2);
    gsl_fft_halfcomplex_wavetable * hc = gsl_fft_halfcomplex_wavetable_alloc (num_pad);
    gsl_fft_real_workspace * work = gsl_fft_real_workspace_alloc (num_pad);
    for(size_t o=0; o<num_obs; o++)
    {
        gsl_fft_halfcomplex_inverse (&ff_coeff(o), 1, num_pad, hc, work);
    }
    gsl_fft_halfcomplex_wavetable_free (hc);
    gsl_fft_real_workspace_free (work);
    ff_coeff = ff_coeff.transpose({1,0});
    tensor<double, 2> ff({num_ts,num_obs});
    ff << ff_coeff[0];
    ff_coeff.clear();
    ff *= sqrt(num_pad);
    ff += ff_average;
    return ff;
}
