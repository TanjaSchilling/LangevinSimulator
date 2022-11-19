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
#include "RandomForceGenerator.hpp"
#include "parameter_handler.hpp"
#include "RK4.hpp"
#include "TensorUtils.hpp"

#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;
using namespace TensorUtils;

int main(int argc, char *argv[]) {

    cout << "BEGIN: main_simulator" << endl;

	size_t num_sim;
	string out_folder;
	bool shift;

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("num_sim: Specify number of simulated trajectories.");
		num_sim = cmdtool.get_int("num_sim", 0);
		cmdtool.add_usage("out_folder: Default: ./OUT");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
		cmdtool.add_usage("shift: Boolean. If true, the average initial value is subtracted. No effect, if false. Default: true");
		shift = cmdtool.get_bool("shift", true);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	filesystem::path path;

	path = out_folder;
	path /= "kernel.f64";
	cout << "Load memory kernel from: " << path << endl;
    tensor<double,4> kernel;
    kernel.read(path);

	path = out_folder;
	path /= "drift.f64";
	cout << "Load drift from: " << path << endl;
    tensor<double,3> drift;
    drift.read(path);

    path = out_folder;
    path /= "ff_average.f64";
	cout << "Load ff_average from: " << path << endl;
	tensor<double> ff_average;
	ff_average.read(path);
	ff_average.reshape({ff_average.size()});
	tensor<double,1> average;
	average = ff_average;

	path = out_folder;
	path /= "cov.f64";
	cout << "Load cov from: " << path << endl;
    tensor<double,2> cov;
    cov.read(path);

	path = out_folder;
	path /= "traj.f64";
	cout << "Load trajectories from: " << path << endl;
    tensor<double,3> trajectories;
    trajectories.read(path);

	path = out_folder;
	path /= "times.f64";
	cout << "Load times from: " << path << endl;
    tensor<double,1> times;
    times.read(path);

    tensor<double,1> average_tau;
    if(shift)
    {
        path = out_folder;
        path /= "average_tau.f64";
        cout << "Load mean inital value from: " << path << endl;
        average_tau.read(path);
    }

    // set args
    double dt = times[1]-times[0];
    size_t num_files = trajectories.shape[0];
    size_t num_ts = trajectories.shape[1];
    size_t num_obs = trajectories.shape[2];
    cout << "Loaded "<< num_files << " trajectories with " << num_ts << " timesteps and " << num_obs << " observables." << endl;

    // covariance matrix from fluctuating forces
    RandomForceGenerator rfg(average, cov, (unsigned)num_obs);
    cov.clear();

    // generate a trajectory
    tensor<double,2> rand_ff;
    tensor<double,2> simulated_trajectory({num_ts,num_obs});
    RK4 rk4;
    path = out_folder;
    path /= "SIM";
    filesystem::create_directories(path);
    for(size_t n=0;n<num_sim; n++){
        rand_ff = rfg.pull_multivariate_gaussian();
        for(size_t i=0; i<num_obs; i++)
        {
            simulated_trajectory(0,i) = trajectories(n%num_files,0,i); // set initial value
        }
        rk4.integrate(dt,drift,kernel,simulated_trajectory,rand_ff);

        if(shift)
        {
            for(size_t t=0; t<num_ts; t++)
            {
                for(size_t o=0; o<num_obs; o++)
                {
                    simulated_trajectory(t,o) += average_tau[o];
                }
            }
        }

        InputOutput::write(times,simulated_trajectory,path/("sim_"+to_string(n)+".txt"));
    }

    cout << "END: main_simulator" << endl << endl;

	return 0;
}
