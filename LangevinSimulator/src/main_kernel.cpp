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

#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <filesystem>

using namespace std;

int main(int argc, char** argv) {

    cout << "BEGIN: main_kernel" << endl;

	string out_folder;
	bool txt_out;
	bool stationary;

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("out_folder: Default: ./OUT");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
        cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
        txt_out = cmdtool.get_bool("txt_out", true);
		cmdtool.add_usage("stationary: Boolean. If true, treats the process as stationary. Default: false.");
		stationary = cmdtool.get_bool("stationary", false);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	cout << "PARAMETERS: " << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;
	cout << "stationary" << '\t'<< stationary << endl;

	filesystem::path out_path = out_folder;

    if(!stationary)
    {
        TensorUtils::tensor<double,4> correlation;
        cout << "Load correlation from: " << out_path/"correlation.f64" << endl;
        correlation.read(out_path/"correlation.f64");

        TensorUtils::tensor<double,1> times;
        cout << "Load times from: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");

        TensorUtils::tensor<double,3> drift;
        try
        {
            cout << "Search drift term: " << out_path/"drift.f64" << endl;
            drift.read(out_path/"drift.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate drift." << endl;
            drift = KernelMethods::getDrift(correlation,times[1]-times[0]);
            cout << "Write drift term: " << out_path/"drift.f64" << endl;
            drift.write("drift.f64",out_path);
            if(txt_out)
            {
                InputOutput::write(times,drift,filesystem::path(out_path/"drift.txt"));
            }
            drift.clear();
        }

        TensorUtils::tensor<double,4> memory_kernel;
        try
        {
            cout << "Search memory kernel: " << out_path/"kernel.f64" << endl;
            memory_kernel.read(out_path/"kernel.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate memory kernel." << endl;
            size_t num_ts = correlation.shape[0];
            size_t num_obs = correlation.shape[0];
            size_t num_tot = num_ts*num_obs;
            gsl_matrix* kernel = gsl_matrix_alloc(num_tot, num_tot);
            gsl_matrix* corr = gsl_matrix_alloc(num_tot, num_tot);
            correlation >> *corr->data;
            KernelMethods::getMemoryKernel(kernel,corr,num_ts,num_obs,times[1]-times[0]);
            gsl_matrix_free(corr);
            memory_kernel.alloc({num_ts,num_obs,num_ts,num_obs});
            memory_kernel << *kernel->data;
            gsl_matrix_free(kernel);
            cout << "Write memory kernel: " << out_path/"kernel.f64" << endl;
            if(txt_out)
            {
                InputOutput::write(times,memory_kernel,out_path/"kernel.txt");
            }
            memory_kernel.transpose({0,2,1,3}).write("kernel.f64",out_path);
        }
    }
    else
    {
        TensorUtils::tensor<double,3> correlation;
        cout << "Load correlation from: " << out_path/"correlation_stationary.f64" << endl;
        correlation.read(out_path/"correlation_stationary.f64");

        TensorUtils::tensor<double,1> times;
        cout << "Load times from: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");
        double dt = times[1]-times[0];

        TensorUtils::tensor<double,2> drift;
        try
        {
            cout << "Search drift term: " << out_path/"drift_stationary.f64" << endl;
            drift.read(out_path/"drift_stationary.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate drift." << endl;
            drift = KernelMethods::getDrift(correlation,dt);
            cout << "Write drift term: " << out_path/"drift_stationary.f64" << endl;
            drift.write("drift_stationary.f64",out_path);
            if(txt_out)
            {
                drift.write("drift_stationary.txt",out_path);
            }
            drift.clear();
        }

        TensorUtils::tensor<double,3> memory_kernel;
        try
        {
            cout << "Search memory kernel: " << out_path/"kernel_stationary.f64" << endl;
            memory_kernel.read(out_path/"kernel_stationary.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate memory kernel." << endl;
            memory_kernel = KernelMethods::getMemoryKernel(correlation,dt);
            cout << "Write memory kernel: " << out_path/"kernel_stationary.f64" << endl;
            if(txt_out)
            {
                TensorUtils::tensor<double,1> new_times({2*times.size()-1});
                for(size_t t=0;t<times.size();t++)
                {
                    new_times[times.size()-1+t] = +dt*t;
                    new_times[times.size()-1-t] = -dt*t;
                }
                InputOutput::write(new_times,memory_kernel,out_path/"kernel_stationary.txt");
            }
            memory_kernel.write("kernel_stationary.f64",out_path);
        }
    }

    cout << "END: main_kernel" << endl << endl;

	return 0;
}
