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

#include "RK4.hpp"

#include <cmath>
#include "KernelMethods.hpp"
#include <gsl/gsl_blas.h>


using namespace std;
using namespace TensorUtils;

RK4::RK4()
{
    //ctor
}

RK4::~RK4()
{
    //dtor
}

tensor<double,1> RK4::f(
    size_t n,
    double dt,
    tensor<double,3> &drift,
    tensor<double,4> &kernel,
    tensor<double,2> &traj,
    tensor<double,2> &rand_ff)
{
    const size_t num_obs = traj.shape[1];
    const size_t num_ts = kernel.shape[0];
    const size_t num_obs_2 = num_obs*num_obs;
    const size_t idx_t1 = n*num_ts*num_obs*num_obs;

    tensor<double,1> f_n_x({num_obs}, 0.0);

    for(size_t tau=0;tau<=n;tau++)
    {
        for(size_t i = 0; i<num_obs; i++)
        {
            buffer_2d(tau,i) = 0.0;
            for(size_t k = 0; k<num_obs; k++)
            {
                buffer_2d[tau*num_obs+i] += kernel[idx_t1+tau*num_obs_2+i*num_obs+k]*traj[tau*num_obs+k];
            }
        }
    }
    size_t j=0;
    while(j+1 < n) // Simpson rule
    {
        for(size_t i=0; i<num_obs; i++)
        {
            f_n_x[i] += buffer_2d[j*num_obs+i];
            f_n_x[i] += 4.0*buffer_2d[(j+1)*num_obs+i];
            f_n_x[i] += buffer_2d[(j+2)*num_obs+i];
        }
        j+=2;
    }
    if(j+1 == n) // Trapezoidal rule for last time-interval
    {
        for(size_t i=0; i<num_obs; i++)
        {
            f_n_x[i] += 1.5*buffer_2d[j*num_obs+i];
            f_n_x[i] += 1.5*buffer_2d[(j+1)*num_obs+i];
        }
    }
    f_n_x *= dt/3.0;

    for(size_t i = 0; i<num_obs; i++)
    {
        for(size_t k = 0; k<num_obs; k++)
        {
            f_n_x[i] += drift[n*num_obs_2+i*num_obs+k]*traj[n*num_obs+k];
        }
        f_n_x[i] += rand_ff[n*num_obs+i];
    }
    return f_n_x;
}

void RK4::process(
    size_t n,
    double dt,
    tensor<double,3> &drift,
    tensor<double,4> &kernel,
    tensor<double,2> &traj,
    tensor<double,2> &rand_ff)
{
    size_t num_obs = traj.shape[1];
    tensor<double,1> k1 = f(n,dt,drift,kernel,traj,rand_ff);
    for(size_t i=0; i<num_obs; i++)
    {
        traj(n+1,i) = traj(n,i)+dt*k1[i];
    }
    tensor<double,1> k2 = f(n+1,dt,drift,kernel,traj,rand_ff);
    for(size_t i=0; i<num_obs; i++)
    {
        traj(n+1,i) = traj(n,i)+dt*k2[i];
    }
    tensor<double,1> k3 = f(n+1,dt,drift,kernel,traj,rand_ff);
    for(size_t i=0; i<num_obs; i++)
    {
        traj(n+2,i) = traj(n,i)+2*dt*k3[i];
    }
    tensor<double,1> k4 = f(n+2,dt,drift,kernel,traj,rand_ff);
    for(size_t i=0; i<num_obs; i++)
    {
        traj(n+2,i) = traj(n,i)+(dt/3.0)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    }
    tensor<double,1> k5 = f(n+2,dt,drift,kernel,traj,rand_ff);
    for(size_t i=0; i<num_obs; i++)
    {
        traj(n+1,i) = 0.5*( traj(n,i)+traj(n+2,i) + dt*(k1[i]-k5[i]) ); // Verlet: one order less: O(dt^4)
    }
}

void RK4::integrate(
    double dt,
    tensor<double,3> &drift,
    tensor<double,4> &kernel,
    tensor<double,2> &traj,
    tensor<double,2> &rand_ff)
{
    buffer_2d.alloc(traj.shape,0.0);
    for(size_t n=0;n+2<traj.shape[0];n+=2)
    {
        process(n,dt,drift,kernel,traj,rand_ff);
    }
    if(traj.shape[0]%2 == 0){
        tensor<double,1> k1 = f(traj.shape[0]-2,dt,drift,kernel,traj,rand_ff);
        for(size_t i=0; i< traj.shape[1]; i++)
        {
            traj(traj.shape[0]-1,i) = traj(traj.shape[0]-2,i)+dt*k1[i];
        }
        tensor<double,1> k2 = f(traj.shape[0]-1,dt,drift,kernel,traj,rand_ff);
        for(size_t i=0; i< traj.shape[1]; i++)
        {
            traj(traj.shape[0]-1,i) = traj(traj.shape[0]-2,i)+0.5*dt*(k1[i]+k2[i]); // Heun
        }
    }
}

