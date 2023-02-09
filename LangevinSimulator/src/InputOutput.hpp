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

#ifndef KERNEL_INPUTOUTPUT_H_
#define KERNEL_INPUTOUTPUT_H_

#include <string>
#include <regex>
#include <gsl/gsl_matrix.h>
#include <filesystem>

#include "TensorUtils.hpp"

/*!
    \brief This namespace provides functions to read/write trajectories from/to text files.
    Square matrices as a function of one or two times may be written as text files.
*/
namespace InputOutput
{
    /*!
        \brief Get paths to the trajectories `<in_folder>/<in_prefix><n>.txt` for each `n` within the given ranges.
        \param file_range Takes the form `0-999` or `0-99,200-299`.
        \param in_folder Path that contains the trajectories to be loaded.
        \param in_prefix Prefix of the desired trajectories.
        \return `vector<string>` with all paths to the desired trajectories.
    */
    std::vector<std::string> getDataFilenames(std::string file_range, std::string in_folder, std::string in_prefix);

    /*!
        \brief Read trajectories for each path in ´data_files´.
        \param data_files Contains the paths obtained from \ref getDataFilenames(std::string file_range, std::string in_folder, std::string in_prefix).
        \param t_min Initial time.
        \param t_max End of desired time interval.
        \param increment Load every `increment`-th time step only.
        \param num_obs Number of observables to be used.
        \return Stores all trajectories in a tensor, i.e. `return_value(n,t,o)`.
        Here, `(n,t,o)` enumerates the trajectories, time-steps and observables, respectively, where `o=0` gives the time of the `t`-th time-step.
    */
    TensorUtils::tensor<double,3> readTrajectories(
        std::vector<std::string> &data_files,
        double t_min,
        double t_max,
        size_t increment,
        size_t num_obs);

    /*!
        \brief Erase the times from the trajectories and return them as a one dimensional tensor instead.
        \param trajectories Contains the trajectories obtained from
        \ref readTrajectories(std::vector<std::string> &data_files,double t_min,double t_max,size_t increment,size_t num_obs).
        \return Times for corresponding trajectories.
    */
    TensorUtils::tensor<double,1> popTimes(TensorUtils::tensor<double,3> &trajectories);

    /*!
        \brief Write a trajectory to a text file. Directory must exist.

        The first column stores the times, the following columns contain the observables.
    */
    void write(TensorUtils::tensor<double,1> &times, TensorUtils::tensor<double,2> &traj, std::filesystem::path out_path);

    /*!
        \brief Write square matrices as a function of two times to a text file. Directory must exist.

        This function is used to write the memory kernel and correlation function to text files.
        The first two columns contain the two times, the following columns contain the matrix elements in lexicographical order.
    */
    void write(TensorUtils::tensor<double,1> &times, TensorUtils::tensor<double,4> &corr, std::filesystem::path out_path);

    /*!
        \brief Write square matrices as a function of time to a text file. Directory must exist.

        This function is used to write the drift term to text files.
        The first column contains the times, the following columns contain the matrix elements in lexicographical order.
    */
    void write(TensorUtils::tensor<double,1> &times, TensorUtils::tensor<double,3> &corr, std::filesystem::path out_path);
}

#endif	// KERNEL_INPUTOUTPUT_H_
