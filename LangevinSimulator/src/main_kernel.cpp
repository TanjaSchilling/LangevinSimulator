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

	cout << "PARAMETERS: " << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;

	filesystem::path out_path = out_folder;
    TensorUtils::tensor<double,4> buffer;

    try
    {
        cout << "Search memory kernel: " << out_path/"kernel.f64" << endl;
        buffer.read(out_path/"kernel.f64");
    }
    catch(exception &ex)
    {
        cout << "Unable to read binary. Calculate memory kernel." << endl;

        TensorUtils::tensor<double,4> correlation;
        cout << "Load correlation from: " << out_path/"correlation.f64" << endl;
        correlation.read(out_path/"correlation.f64");

        TensorUtils::tensor<double,1> times;
        cout << "Load times from: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");

        size_t num_ts = correlation.shape[0];
        size_t num_obs = correlation.shape[1];
        size_t num_tot = num_ts * num_obs;
        double dt = (times[1]-times[0]);

        std::cout << "#\tnum_ts: " << num_ts << "\tnum_obs: " << num_obs << "\n";
        std::cout << "#\tdt: " << dt << "\n";

        gsl_matrix* kernel = gsl_matrix_alloc(num_tot, num_tot);
        gsl_matrix* corr = gsl_matrix_alloc(num_tot, num_tot);
        correlation >> *corr->data;

        KernelMethods::getMemoryKernel(kernel,corr,num_ts,num_obs,dt);

        gsl_matrix_free(corr);
        buffer.alloc({num_ts,num_obs,num_ts,num_obs});
        buffer << *kernel->data;
        gsl_matrix_free(kernel);
        cout << "Write memory kernel: " << out_path/"kernel.f64" << endl;
        if(txt_out)
        {
            InputOutput::write(times,buffer,out_path/"kernel.txt");
        }
        buffer.transpose({0,2,1,3}).write("kernel.f64",out_path);
        buffer.clear();
    }

    cout << "END: main_kernel" << endl << endl;

	return 0;
}
