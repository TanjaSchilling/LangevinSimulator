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

#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <filesystem>

#include "parameter_handler.hpp"
#include "InputOutput.hpp"
#include "KernelMethods.hpp"

#include "TensorUtils.hpp"

using namespace std;

int main(int argc, char** argv) {

    cout << "BEGIN: main_kernel" << endl;

	double dt;
	size_t num_ts;  // Number of time steps
	size_t num_obs;  // Number of observables
	size_t num_tot;  // Number of time steps times number of observables
	string out_folder;
	bool txt_out;

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("num_obs: Number of observables.");
		num_obs = cmdtool.get_int("num_obs", 0);
		cmdtool.add_usage("out_folder: Default: ./OUT");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
        cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
        txt_out = cmdtool.get_bool("txt_out", true);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	filesystem::path path;
    TensorUtils::tensor<double,4> buffer;

    try
    {
        path = out_folder;
        path /= "kernel.f64";
        cout << "Search memory kernel: " << path << endl;
        buffer.read(path);
    }
    catch(exception &ex)
    {
        cout << "Unable to read binary. Calculate memory kernel." << endl;

        path = out_folder;
        path /= "correlation.f64";
        cout << "Load correlation from: " << path << endl;
        TensorUtils::tensor<double,4> correlation;
        correlation.read(path);

        path = out_folder;
        path /= "times.f64";
        cout << "Load times from: " << path << endl;
        TensorUtils::tensor<double,1> times;
        times.read(path);

        num_ts = correlation.shape[0];
        num_obs = correlation.shape[1];
        num_tot = num_ts * num_obs;
        dt = (times[1]-times[0]);

        std::cout << "#\tnum_ts: " << num_ts << "\tnum_obs: " << num_obs << "\n";
        std::cout << "#\tdt: " << dt << "\n";

        // Until last step, K is used as auxiliary matrix for calculations
        gsl_matrix* kernel = gsl_matrix_alloc(num_tot, num_tot);
        gsl_matrix* corr = gsl_matrix_alloc(num_tot, num_tot);
        correlation >> *corr->data;

        KernelMethods::getMemoryKernel(kernel,corr,num_ts,num_obs,dt);

        gsl_matrix_free(corr);
        buffer.alloc({num_ts,num_obs,num_ts,num_obs});
        buffer << *kernel->data;
        gsl_matrix_free(kernel);
        if(txt_out)
        {
            path = out_folder;
            path /= "kernel.txt";
            InputOutput::write(times,buffer,path);
        }
        buffer.transpose({0,2,1,3}).write("kernel.f64",out_folder);
        buffer.clear();
    }

    cout << "END: main_kernel" << endl << endl;

	return 0;
}
