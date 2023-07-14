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

#include "InputOutput.hpp"
#include "KernelMethods.hpp"

#include "parameter_handler.hpp"
#include "TensorUtils.hpp"

#include <vector>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace TensorUtils;

int main(int argc, char *argv[]) {

    cout << "BEGIN: main_fluctuating_forces" << endl;

	string out_folder;
	bool txt_out;
	bool gaussian_init_val;
	bool darboux_sum;
	bool stationary;

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("out_folder: Default: ./OUT");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
        cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
        txt_out = cmdtool.get_bool("txt_out", true);
        cmdtool.add_usage("gaussian_init_val: Boolean. If true, the initial values will be drawn from a Gaussian. \
                          Else, the original initial values will be used for numerical simulations. Default: false");
        gaussian_init_val = cmdtool.get_bool("gaussian_init_val", false);
        cmdtool.add_usage("darboux_sum: Boolean. If true, the forward difference quotient and lower Darboux sum \
                          are used for the calculation of the fluctuating forces and the numerical integration. \
                          Else, the symmetric difference quotient and Simpson rule are used. Default: true");
        darboux_sum = cmdtool.get_bool("darboux_sum", true);
		cmdtool.add_usage("stationary: Boolean. If true, treats the process as stationary. Default: false.");
		stationary = cmdtool.get_bool("stationary", false);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	cout << "PARAMETERS: " << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;
	cout << "gaussian_init_val" << '\t'<< gaussian_init_val << endl;
	cout << "darboux_sum" << '\t' << darboux_sum << endl;
	cout << "stationary" << '\t' << stationary << endl;


	filesystem::path out_path = out_folder;

    if(!stationary)
    {
        tensor<double,3> fluctuating_force;
        try
        {
            cout << "Search fluctuating forces: " << out_path/"ff.f64" << endl;
            fluctuating_force.read(out_path/"ff.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate fluctuating forces." << endl;

            cout << "Load memory kernel from: " << out_path/"kernel.f64" << endl;
            tensor<double,4> kernel;
            kernel.read(out_path/"kernel.f64");

            cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
            tensor<double,3> trajectories;
            trajectories.read(out_path/"traj.f64");

            cout << "Load times from: " << out_path/"times.f64" << endl;
            tensor<double,1> times;
            times.read(out_path/"times.f64");

            cout << "Search drift term: " << out_path/"drift.f64" << endl;
            tensor<double,3> drift;
            drift.read(out_path/"drift.f64");

            // calculate fluctuating forces
            fluctuating_force = KernelMethods::getFluctuatingForce(kernel, drift, trajectories, times, darboux_sum);

            cout << "Write fluctuating forces." << endl;
            fluctuating_force.write("ff.f64",out_path);
            if(txt_out)
            {
                filesystem::create_directories(out_path/"FF");
                tensor<double,2> ff({fluctuating_force.shape[1],fluctuating_force.shape[2]});
                for(size_t n=0; n<fluctuating_force.shape[0]; n++)
                {
                    ff << fluctuating_force(n);
                    InputOutput::write(times,ff,(out_path/"FF")/("ff_"+to_string(n)+".txt"));
                }
            }
        }
        tensor<double,4> ff_cov;
        tensor<double,2> ff_average;
        try
        {
            cout << "Search covariance of fluctuating forces: " << out_path/"ff_cov.f64" << endl;
            ff_cov.read(out_path/"ff_cov.f64");
            cout << "Search average of fluctuating forces: " << out_path/"ff_average.f64" << endl;
            ff_average.read(out_path/"ff_average.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary." << endl;
            if(!gaussian_init_val)
            {
                cout << "Compute average and covariance matrix of fluctuating forces." << endl;
                KernelMethods::writeCovarianceMatrix(fluctuating_force, out_path);
            }
            else
            {
                cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
                tensor<double,3> trajectories;
                trajectories.read(out_path/"traj.f64");

                cout << "Compute average and covariance matrix of initial values and fluctuating forces." << endl;
                KernelMethods::writeExtendedCovarianceMatrix(trajectories, fluctuating_force, out_path);
            }
        }
    }
    else
    {
        tensor<double,3> fluctuating_force;
        try
        {
            cout << "Search fluctuating forces: " << out_path/"ff.f64" << endl;
            fluctuating_force.read(out_path/"ff.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate fluctuating forces." << endl;

            cout << "Load memory kernel from: " << out_path/"kernel_stationary.f64" << endl;
            tensor<double,3> kernel;
            kernel.read(out_path/"kernel_stationary.f64");

            cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
            tensor<double,3> trajectories;
            trajectories.read(out_path/"traj.f64");

            cout << "Load times from: " << out_path/"times.f64" << endl;
            tensor<double,1> times;
            times.read(out_path/"times.f64");

            cout << "Search drift term: " << out_path/"drift_stationary.f64" << endl;
            tensor<double,2> drift;
            drift.read(out_path/"drift_stationary.f64");

            // calculate fluctuating forces
            fluctuating_force = KernelMethods::getFluctuatingForce(kernel, drift, trajectories, times, darboux_sum);

            cout << "Write fluctuating forces." << endl;
            fluctuating_force.write("ff.f64",out_path);
            if(txt_out)
            {
                filesystem::create_directories(out_path/"FF");
                tensor<double,2> ff({fluctuating_force.shape[1],fluctuating_force.shape[2]});
                for(size_t n=0; n<fluctuating_force.shape[0]; n++)
                {
                    ff << fluctuating_force(n);
                    InputOutput::write(times,ff,(out_path/"FF")/("ff_"+to_string(n)+".txt"));
                }
            }
        }
        tensor<double,3> ff_cov;
        tensor<double,2> ff_average;
        try
        {
            cout << "Search covariance of fluctuating forces: " << out_path/"ff_cov_stationary.f64" << endl;
            ff_cov.read(out_path/"ff_cov.f64");
            cout << "Search average of fluctuating forces: " << out_path/"ff_average.f64" << endl;
            ff_average.read(out_path/"ff_average.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary." << endl;
            if(!gaussian_init_val)
            {
                cout << "Compute average and covariance matrix of fluctuating forces." << endl;
                KernelMethods::writeCovarianceMatrix(fluctuating_force, out_path, true);
            }
            else
            {
                cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
                tensor<double,3> trajectories;
                trajectories.read(out_path/"traj.f64");

                cout << "Compute average and covariance matrix of initial values and fluctuating forces." << endl;
                KernelMethods::writeExtendedCovarianceMatrix(trajectories, fluctuating_force, out_path, true);
            }
        }
    }

    cout << "END: main_fluctuating_forces" << endl << endl;

	return 0;
}
