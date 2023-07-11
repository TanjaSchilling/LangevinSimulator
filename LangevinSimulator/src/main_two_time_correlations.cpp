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
#include "FourierTransforms.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <set>
#include <sstream>
#include <cmath>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_math.h>


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
	size_t mollifier_width;
	bool stationary;
	size_t chop_stationary_trajectories;

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
		cmdtool.add_usage("mollifier_width: unsigned integer. Total width of the mollifier is 2*<mollifier_width> time-steps. \
                    No effect, if <mollifier_width> is set to zero. If <mollifier_width> is positive, all input trajectories will be mollified.");
		cmdtool.add_usage("stationary: Boolean. If true, treats the process as stationary. Default: false.");
		cmdtool.add_usage("chop_stationary_trajectories: Unsigned integer. No effect, if <chop_stationary_trajectories>=0 or <stationary>=false. \
                    Chops the trajectories into smaller pieces with size <chop_stationary_trajectories>, \
                    but uses the 'unchopped' trajectories to compute the correlation function. Default: 0.");
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
		mollifier_width = cmdtool.get_int("mollifier_width", 0);
		stationary = cmdtool.get_bool("stationary", false);
		chop_stationary_trajectories = cmdtool.get_int("chop_stationary_trajectories", 0);

	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	cout << "PARAMETERS: " << endl;
	cout << "in_folder" << '\t'<< in_folder << endl;
//	cout << "in_prefix" << '\t'<< in_prefix << endl;
//	cout << "increment" << '\t'<< increment << endl;
//	cout << "t_min" << '\t'<< t_min << endl;
//	cout << "t_max" << '\t'<< t_max << endl;
//	cout << "file_range" << '\t'<< file_range << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "shift" << '\t'<< shift << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;
	cout << "mollifier_width" << '\t'<< mollifier_width << endl;
	cout << "stationary" << '\t'<< stationary << endl;

	filesystem::path out_path = out_folder;
	filesystem::path in_path = in_folder;

    /**
        TRAJECTORIES AND TIMES
    **/
	TensorUtils::tensor<double,3> traj;
    TensorUtils::tensor<double,1> times;
    vector<size_t> unchopped_shape;
    vector<size_t> chopped_shape;

    try
    {
        cout << "Search trajectories: " << out_path/"traj.f64" << endl;
        traj.read(out_path/"traj.f64");
        cout << "Search times: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");
    }
    catch(exception &ex)
    {
        try
        {
            cout << "Search trajectories: " << in_path/"traj.f64" << endl;
            traj.read(in_path/"traj.f64");
            cout << "Search times: " << in_path/"times.f64" << endl;
            times.read(in_path/"times.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binaries. Read trajectories from text files." << endl;
            vector<string> data_files = InputOutput::getDataFilenames(file_range, in_path, in_prefix);
            traj = InputOutput::readTrajectories(data_files,t_min,t_max,increment,num_obs);
            times = InputOutput::popTimes(traj);
        }
        if(mollifier_width>1)
        {
            cout << "Mollify trajectories." << endl;
            KernelMethods::mollifyTrajectories(times,traj,mollifier_width);
        }
        if(stationary)
        {
            if(chop_stationary_trajectories>0)
            {
                if(chop_stationary_trajectories>traj.shape[1])
                {
                    chop_stationary_trajectories=0;
                }
                else
                {
                    TensorUtils::tensor<double,3> buffer({traj.shape[0],traj.shape[1]-traj.shape[1]%chop_stationary_trajectories,traj.shape[2]});
                    for(size_t n=0; n< buffer.shape[0]; n++)
                    {
                        for(size_t t=0; t<buffer.shape[1]; t++)
                        {
                            for(size_t o=0; o<buffer.shape[2]; o++)
                            {
                                buffer(n,t,o) = traj(n,t,o);
                            }
                        }
                    }
                    traj = buffer;
                    buffer.clear();
                    unchopped_shape = traj.shape;
                    chopped_shape = {traj.shape[0]*(traj.shape[1]/chop_stationary_trajectories),chop_stationary_trajectories,traj.shape[2]};
                    traj.reshape(chopped_shape);
                    TensorUtils::tensor<double,1> new_times({traj.shape[1]});
                    new_times << times[0];
                    times = new_times;
                    new_times.clear();
                }
            }
        }
        if(shift)
        {
            cout << "Shift trajectories and write mean initial values: " << out_path/"mean_initial_value.f64" << endl;
            TensorUtils::tensor<double,1> mean_initial_value = KernelMethods::shiftTrajectories(traj);
            mean_initial_value.write("mean_initial_value.f64",out_path);
            if(txt_out)
            {
                mean_initial_value.write("mean_initial_value.txt",out_path);
            }
        }
        cout << "Write trajectories: " << out_path/"traj.f64" << endl;
        traj.write("traj.f64",out_path);
        cout << "Write times: " << out_path/"times.f64" << endl;
        times.write("times.f64",out_path);
    }

    /**
        CORRELATION FUNCTION
    **/
    if(!stationary)
    {
        TensorUtils::tensor<double,4> correlation;
        try
        {
            cout << "Search correlation function: " << out_path/"correlation.f64" << endl;
            correlation.read(out_path/"correlation.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate correlation function." << endl;
            correlation = KernelMethods::getCorrelationFunction(traj);
            cout << "Write correlation function: " << out_path/"correlation.f64" << endl;
            correlation.write("correlation.f64",out_path);
            if(txt_out)
            {
                InputOutput::write(times,correlation,out_path/"correlation.txt");
            }
        }
    }
    else
    {
        TensorUtils::tensor<double,3> correlation;
        try
        {
            cout << "Search correlation function: " << out_path/"correlation_stationary.f64" << endl;
            correlation.read(out_path/"correlation_stationary.f64");
        }
        catch(exception &ex)
        {
            cout << "Unable to read binary. Calculate correlation function." << endl;
            if(chopped_shape.size()>0)
            {
                traj.reshape(unchopped_shape);
            }
            correlation = KernelMethods::getStationaryCorrelation(traj);
            if(chopped_shape.size()>0)
            {
                traj.reshape(chopped_shape);
                TensorUtils::tensor<double,3> buffer({2*traj.shape[1]-1,traj.shape[2],traj.shape[2]});
                for(size_t i=0; i<traj.shape[2]; i++)
                {
                    for(size_t j=0; j<traj.shape[2]; j++)
                    {
                        for(int tau=1-traj.shape[1]; tau<(int)traj.shape[1]; tau++)
                        {
                            buffer(traj.shape[1]-1+tau,i,j) = correlation(unchopped_shape[1]-1+tau,i,j);
                        }
                    }
                }
                correlation = buffer;
                buffer.clear();
            }
            cout << "Write correlation function: " << out_path/"correlation_stationary.f64" << endl;
            correlation.write("correlation_stationary.f64",out_path);
            if(txt_out)
            {
                double dt = times[1]-times[0];
                TensorUtils::tensor<double,1> new_times({2*times.size()-1});
                for(size_t t=0;t<times.size();t++)
                {
                    new_times[times.size()-1+t] = +dt*t;
                    new_times[times.size()-1-t] = -dt*t;
                }
                InputOutput::write(new_times,correlation,out_path/"correlation_stationary.txt");
            }
        }
    }

    cout << "END: main_two_time_correlations" << endl << endl;

	return 0;
};
