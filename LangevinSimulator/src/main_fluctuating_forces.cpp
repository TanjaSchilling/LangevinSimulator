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

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("out_folder: Default: ./OUT");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
        cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
        txt_out = cmdtool.get_bool("txt_out", true);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	filesystem::path path;
    tensor<double,3> fluctuating_force;

    try
    {
        path = out_folder;
        path /= "ff.f64";
        cout << "Search fluctuating forces: " << path << endl;
        fluctuating_force.read(path);
    }
    catch(exception &ex)
    {
        cout << "Unable to read binary. Calculate fluctuating forces." << endl;

        path = out_folder;
        path /= "kernel.f64";
        cout << "Load memory kernel from: " << path << endl;
        tensor<double,4> kernel;
        kernel.read(path);

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

        // set args
        size_t num_files = trajectories.shape[0];
        size_t num_ts = trajectories.shape[1];
        size_t num_obs = trajectories.shape[2];
        cout << "Loaded "<< num_files << " trajectories with " << num_ts << " timesteps and " << num_obs << " observables." << endl;

        // calculate fluctuating forces
        fluctuating_force = KernelMethods::getFluctuatingForce(kernel, trajectories, times, out_folder, txt_out);
        KernelMethods::writeCovarianceMatrix(fluctuating_force, out_folder);

        // write fluctuating forces
        //InputOutput::writeFluctuatingForces(fluctuating_force, times, out_folder);
        fluctuating_force.write("ff.f64",out_folder);
        if(txt_out)
        {
            path=out_folder;
            path/="FF";
            filesystem::create_directories(path);
            tensor<double,2> ff({fluctuating_force.shape[1],fluctuating_force.shape[2]});
            for(size_t n=0; n<fluctuating_force.shape[0]; n++)
            {
                ff << fluctuating_force[n];
                InputOutput::write(times,ff,path/("ff_"+to_string(n)+".txt"));
            }
        }
    }

    cout << "END: main_fluctuating_forces" << endl << endl;

	return 0;
}
