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

#include<iostream>
#include<fstream>
#include<string>
#include<filesystem>
#include<vector>
#include<set>
#include<sstream>
#include <cmath>

#include"parameter_handler.hpp"
#include"InputOutput.hpp"

#include "TensorUtils.hpp"

using namespace std;

int main(int argc, char** argv) {

    cout << "BEGIN: main_two_time_correlations" << endl;

	string in_folder;
	string in_prefix;
	size_t num_obs;
	size_t increment;
	double t_min;
	double t_max;
	string file_range;
	string out_folder;
	bool shift;
	bool txt_out;

	ParameterHandler cmdtool {argc, argv};
	cmdtool.process_flag_help();
	cmdtool.process_parameters();
	try{
		cmdtool.add_usage("in_folder: Path to dir containing data-files.");
		cmdtool.add_usage("in_prefix: Prefix of data-files.");
		cmdtool.add_usage("t_min: Time to begin evaluation.");
		cmdtool.add_usage("t_max: Time to end evaluation.");
		cmdtool.add_usage("num_obs: Number of observables.");
		cmdtool.add_usage("increment: Use every <increment>-th time step.");
		cmdtool.add_usage("file_range: Specify desired ranges of file numbers, e.g. 1-100 or 101-200,301-400 .");
		cmdtool.add_usage("out_folder: Default: ./OUT");
		cmdtool.add_usage("shift: Boolean. If true, the average initial value is subtracted. No effect, if false. Default: true");
		cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
		// TODO parameters for choosing a certain file set

		in_folder = cmdtool.get_string("in_folder","./TEST_DATA");
		in_prefix = cmdtool.get_string("in_prefix","");
		t_min = cmdtool.get_double("t_min", 0.);
		t_max = cmdtool.get_double("t_max", 1000.);
		increment = cmdtool.get_int("increment", 1);
		num_obs = cmdtool.get_int("num_obs", 0);
		file_range = cmdtool.get_string("file_range", "0-999");
		out_folder = cmdtool.get_string("out_folder", "./OUT");
		shift = cmdtool.get_bool("shift", true);
		txt_out = cmdtool.get_bool("txt_out", true);
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	filesystem::path path;

    /**
        TRAJECTORIES AND TIMES
    **/
	TensorUtils::tensor<double,3> traj;
    TensorUtils::tensor<double,1> times;

    try
    {
        path = out_folder;
        path /= "traj.f64";
        cout << "Search trajectories: " << path << endl;
        traj.read(path);

        path = out_folder;
        path /= "times.f64";
        cout << "Search times: " << path << endl;
        times.read(path);
    }
    catch(exception &ex)
    {
        cout << "Unable to read binaries. Read trajectories from text files." << endl;
        vector<string> dataFiles = InputOutput::getDataFilenames(file_range, in_folder, in_prefix);
        if(dataFiles.size()==0) {
            cout << "Unable to load input files!. -> return." << endl;
            return 0;
        }
        // read
        traj = InputOutput::readTrajectories(dataFiles,t_min,t_max,increment,num_obs);
        times = InputOutput::popTimes(traj);
        if(shift)
        {
            TensorUtils::tensor<double,1> average_tau({num_obs},0.0);
            for(size_t n=0; n<traj.shape[0]; n++)
            {
                for(size_t o=0; o<num_obs; o++)
                {
                    average_tau[o] += traj(n,0,o);
                }
            }
            average_tau /= traj.shape[0];
            average_tau.write("average_tau.f64",out_folder);
            if(txt_out)
            {
                average_tau.write("average_tau.txt",out_folder);
            }
            for(size_t n=0; n<traj.shape[0]; n++)
            {
                for(size_t t=0; t<traj.shape[1]; t++)
                {
                    for(size_t o=0; o<num_obs; o++)
                    {
                        traj(n,t,o) -= average_tau[o];
                    }
                }
            }
        }
        traj.write("traj.f64",out_folder);
        times.write("times.f64",out_folder);
    }

    // set dimensions
    size_t num_files = traj.shape[0];
    size_t num_ts = traj.shape[1];
    num_obs = traj.shape[2];

	cout << "# Found " << num_files << " trajectories with " << num_ts << " time-steps and " << num_obs << " observables." << endl;

    /**
        CORRELATION FUNCTION
    **/

    TensorUtils::tensor<double,4> correlation;

    try
    {
        path = out_folder;
        path /= "correlation.f64";
        cout << "Search correlation function: " << path << endl;
        correlation.read(path);
    }
    catch(exception &ex)
    {
        cout << "Unable to read binary. Calculate correlation function." << endl;

        size_t num_tot = num_ts*num_obs;
        correlation.alloc({num_ts,num_obs,num_ts,num_obs});
        double * result = &correlation[0];
        traj *= sqrt(1.0/num_files);
        double dummy;
        for(size_t i=0; i<num_tot; i++)
        {
            for(size_t j=0; j<num_tot; j++)
            {
                dummy = 0.0;
                for(size_t n=0; n<num_files; n++)
                {
                    dummy += traj[n*num_tot+i]*traj[n*num_tot+j];
                }
                *result = dummy;
                result++;
            }
        }
        correlation.write("correlation.f64",out_folder);
        if(txt_out)
        {
            path = out_folder;
            path /= "correlation.txt";
            InputOutput::write(times,correlation,path);
        }
    }

    cout << "END: main_two_time_correlations" << endl << endl;

	return 0;
};
