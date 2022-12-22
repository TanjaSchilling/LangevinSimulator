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
            vector<string> dataFiles = InputOutput::getDataFilenames(file_range, in_path, in_prefix);
            if(dataFiles.size()==0) {
                cout << "Unable to load input files!. -> return." << endl;
                return 0;
            }
            // read
            traj = InputOutput::readTrajectories(dataFiles,t_min,t_max,increment,num_obs);
            times = InputOutput::popTimes(traj);
        }

        if(shift)
        {
            cout << "Shift trajectories." << endl;
            TensorUtils::tensor<double,1> mean_initial_value({num_obs},0.0);
            for(size_t n=0; n<traj.shape[0]; n++)
            {
                for(size_t o=0; o<num_obs; o++)
                {
                    mean_initial_value[o] += traj(n,0,o);
                }
            }
            mean_initial_value /= traj.shape[0];
            cout << "Write mean initial values: " << out_path/"mean_initial_value.f64" << endl;
            mean_initial_value.write("mean_initial_value.f64",out_path);
            if(txt_out)
            {
                mean_initial_value.write("mean_initial_value.txt",out_path);
            }
            for(size_t n=0; n<traj.shape[0]; n++)
            {
                for(size_t t=0; t<traj.shape[1]; t++)
                {
                    for(size_t o=0; o<num_obs; o++)
                    {
                        traj(n,t,o) -= mean_initial_value[o];
                    }
                }
            }
        }
        if(mollifier_width>1)
        {
            cout << "Mollify trajectories." << endl;
            size_t num_traj = traj.shape[0];
            size_t num_ts = traj.shape[1];
            size_t num_obs = traj.shape[2];

            size_t num_pad = num_ts + 2*mollifier_width+1 -1;
            while(num_pad&(num_pad-1)) // num_pad is not a power of 2!
            {
                num_pad++;
            }

            TensorUtils::tensor<double> traj_pad({num_traj,num_obs,num_pad},0.0);
            for(size_t n=0; n<num_traj; n++)
            {
                for(size_t o=0;o<num_obs;o++)
                {
                    for(size_t t=0;t<num_ts;t++)
                    {
                        traj_pad(n,o,t) = traj(n,t,o);
                    }
                }
            }
            traj.clear();

            TensorUtils::tensor<double> mollifier({num_pad},0.0);
            double sum = 0.0;
            for(size_t t=1; t<2*mollifier_width; t++)
            {
                mollifier[t] = exp( 1/(pow(double(t)/mollifier_width-1,2)-1) );
                sum += mollifier[t];
            }
            mollifier *= 2.0/(num_pad*sum);

            double * lookup_table = new double[num_pad];
            FFTBW::FourierTransforms<double>::initLookUp(lookup_table,num_pad);
            FFTBW::FourierTransforms<double>::fftReal(&mollifier[0],num_pad,+1,lookup_table,false);
            for(size_t n=0; n<num_traj; n++)
            {
                for(size_t o=0;o<num_obs;o++)
                {
                    FFTBW::FourierTransforms<double>::convolve(&traj_pad(n,o),&mollifier[0],num_pad,lookup_table,false);
                }
            }
            delete[] lookup_table;

            traj.alloc({num_traj,num_ts-2*mollifier_width,num_obs});
            for(size_t n=0; n<num_traj; n++)
            {
                for(size_t t=0;t<num_ts-2*mollifier_width;t++)
                {
                    for(size_t o=0;o<num_obs;o++)
                    {
                        traj(n,t,o) = traj_pad(n,o,t+2*mollifier_width);
                    }
                }
            }
            traj_pad.clear();
            TensorUtils::tensor<double> new_times({num_ts-2*mollifier_width});
            new_times << times[mollifier_width];
            times = new_times;
            new_times.clear();
        }
        cout << "Write trajectories: " << out_path/"traj.f64" << endl;
        traj.write("traj.f64",out_path);
        cout << "Write times: " << out_path/"times.f64" << endl;
        times.write("times.f64",out_path);
    }

    // set dimensions
    size_t num_traj = traj.shape[0];
    size_t num_ts = traj.shape[1];
    num_obs = traj.shape[2];

	cout << "# Found " << num_traj << " trajectories with " << num_ts << " time-steps and " << num_obs << " observables." << endl;

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
            gsl_matrix * in_buffer = gsl_matrix_alloc(num_traj,num_ts*num_obs);
            traj *= sqrt(1.0/num_traj);
            traj >> *in_buffer->data;
            traj.clear();
            gsl_matrix * out_buffer = gsl_matrix_alloc(num_ts*num_obs,num_ts*num_obs);
            gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, in_buffer, in_buffer, 0.0, out_buffer);
            gsl_matrix_free(in_buffer);
            correlation.alloc({num_ts,num_obs,num_ts,num_obs});
            correlation << *out_buffer->data;
            gsl_matrix_free(out_buffer);
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

            size_t num_pad = 2*num_ts-1;
            while(num_pad&(num_pad-1)) // num_pad is not a power of 2!
            {
                num_pad++;
            }
            TensorUtils::tensor<double> traj_pad({num_traj,num_obs,num_pad},0.0);
            for(size_t n=0; n<num_traj; n++)
            {
                for(size_t o=0;o<num_obs;o++)
                {
                    for(size_t t=0;t<num_ts;t++)
                    {
                        traj_pad(n,o,t) = traj(n,t,o);
                    }
                }
            }
            //traj.clear();
            double * lookup_table = new double[num_pad];
            FFTBW::FourierTransforms<double>::initLookUp(lookup_table,num_pad);
            for(size_t n=0; n<num_traj; n++)
            {
                for(size_t o=0;o<num_obs;o++)
                {
                    FFTBW::FourierTransforms<double>::fftReal(&traj_pad(n,o),num_pad,+1,lookup_table,false);
                }
            }
            correlation.alloc({num_obs,num_obs,num_pad},0.0);
            for(size_t n=0;n<num_traj;n++)
            {
                for(size_t o1=0;o1<num_obs;o1++)
                {
                    for(size_t o2=0;o2<num_obs;o2++)
                    {
                        correlation(o1,o2,0) += traj_pad(n,o1,0)*traj_pad(n,o2,0);
                        correlation(o1,o2,1) += traj_pad(n,o1,1)*traj_pad(n,o2,1);
                        for(size_t k=2;k<num_pad;k+=2)
                        {
                            correlation(o1,o2,k) += traj_pad(n,o1,k)*traj_pad(n,o2,k)+traj_pad(n,o1,k+1)*traj_pad(n,o2,k+1);
                            correlation(o1,o2,k+1) +=  -1*traj_pad(n,o1,k)*traj_pad(n,o2,k+1)+traj_pad(n,o1,k+1)*traj_pad(n,o2,k);
                        }
                    }
                }
            }
            for(size_t o1=0;o1<num_obs;o1++)
            {
                for(size_t o2=0;o2<num_obs;o2++)
                {
                    FFTBW::FourierTransforms<double>::fftReal(&correlation(o1,o2),num_pad,-1,lookup_table,false);
                }
            }
            delete[] lookup_table;
            correlation=correlation.transpose({2,0,1});
            double dummy;
            for(size_t t=0;t<num_ts;t++)
            {
                dummy = 1.0/(num_ts-t);
                for(size_t i=0;i<num_obs;i++)
                {
                    for(size_t j=0;j<num_obs;j++)
                    {
                        correlation(t,i,j) *= dummy;
                    }
                }
            }
            for(size_t t=1;t<num_ts;t++)
            {
                dummy = 1.0/(num_ts-t);
                for(size_t i=0;i<num_obs;i++)
                {
                    for(size_t j=0;j<num_obs;j++)
                    {
                        correlation(num_pad-t,i,j) *= dummy;
                    }
                }
            }
            correlation *= 2.0/(num_pad*num_traj);
            TensorUtils::tensor<double,3> corr_out({2*num_ts-1,num_obs,num_obs});
            for(size_t tau=0;tau<num_ts;tau++)
            {
                for(size_t k=0;k<num_obs;k++)
                {
                    for(size_t l=0;l<num_obs;l++)
                    {
                        corr_out(num_ts-1+tau,k,l) = correlation(tau,k,l);
                    }
                }
            }
            for(size_t tau=1;tau<num_ts;tau++)
            {
                for(size_t k=0;k<num_obs;k++)
                {
                    for(size_t l=0;l<num_obs;l++)
                    {
                        corr_out(num_ts-1-tau,k,l) = correlation(num_pad-tau,k,l);
                    }
                }
            }
            cout << "Write correlation function: " << out_path/"correlation_stationary.f64" << endl;
            corr_out.write("correlation_stationary.f64",out_path);
        }
    }

    cout << "END: main_two_time_correlations" << endl << endl;

	return 0;
};
