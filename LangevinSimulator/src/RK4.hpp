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

#ifndef RK4_HPP
#define RK4_HPP

#include <vector>

#include "TensorUtils.hpp"

/*!
    \brief This class offers routines to integrate the nsGLE using the classical Runge-Kutta method (RK4).
*/
class RK4
{
    public:
        RK4();
        virtual ~RK4();

        /*!
            \brief Integrates the equation of motions for a given realization of the fluctuating force `rand_ff` using the classical Runge-Kutta method.

            The initial value must be stored in `traj[0]`. The times must be equally spaced.
            The length of `traj` determines the length of the simulated trajectory.
        */
        void integrate(
            double dt,
            TensorUtils::tensor<double,3> &drift,
            TensorUtils::tensor<double,4> &kernel,
            TensorUtils::tensor<double,2> &traj,
            TensorUtils::tensor<double,2> &rand_ff);

    protected:

        //! \brief Returns the time-derivative of the observable `A'(t)=:f(t,A(t))`, where `t=n*dt`.
        TensorUtils::tensor<double,1> f(
            size_t n,
            double dt,
            TensorUtils::tensor<double,3> &drift,
            TensorUtils::tensor<double,4> &kernel,
            TensorUtils::tensor<double,2> &traj,
            TensorUtils::tensor<double,2> &rand_ff);

        /*!
            \brief Executes a classcal Runge-Kutta step with step-size `2dt`.

            Odd times are restored using the Verlet scheme. Local error is in `O(h**4)` instead of `O(h**5)`.
        */
        void process(
            size_t n,
            double dt,
            TensorUtils::tensor<double,3> &drift,
            TensorUtils::tensor<double,4> &kernel,
            TensorUtils::tensor<double,2> &traj,
            TensorUtils::tensor<double,2> &rand_ff);

        //! \private
        TensorUtils::tensor<double,1> buffer;
        //! \private
        TensorUtils::tensor<double,2> buffer_2d;
};

#endif // RK4_HPP
