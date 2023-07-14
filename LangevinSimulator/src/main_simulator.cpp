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
#include <vector>
#include <filesystem>

#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>

using namespace std;
using namespace TensorUtils;

#include "FourierTransforms.hpp"

int main(int argc, char *argv[]) {

    cout << "BEGIN: main_simulator" << endl;

//    tensor<double> test({64}, 1.0);
//    double * lookup = new double[64];
//    for(size_t n=0;n<64;n++)
//    {
//        test[n] += std::sin(2*M_PI*n/64);
//    }
//    FFTBW::FourierTransforms<double>::initLookUp(lookup,64);
//    FFTBW::FourierTransforms<double>::fftReal(&test[0],64,+1,lookup,false);
//    //FFTBW::FourierTransforms<double>::fftReal(&test[0],64,-1,lookup,false);
//    for(size_t n=0;n<64;n++)
//    {
//        cout << test[n] << endl;
//    }
//    delete[] lookup;
//    return 0;

	size_t num_sim;
	string out_folder;
	bool shift;
	bool txt_out;
	bool gaussian_init_val;
	bool darboux_sum;
	bool stationary;
	bool accelerate_stationary_decomp;

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
		cmdtool.add_usage("txt_out: Boolean. If true, writes output files in text format. Default: true");
		txt_out = cmdtool.get_bool("txt_out", true);
        cmdtool.add_usage("gaussian_init_val: Boolean. If true, the initial values will be drawn from a Gaussian. \
                          Else, the original initial values will be used for numerical simulations. Default: false");
        gaussian_init_val = cmdtool.get_bool("gaussian_init_val", false);
		txt_out = cmdtool.get_bool("txt_out", true);
        cmdtool.add_usage("darboux_sum: Boolean. If true, the forward difference quotient and lower Darboux sum \
                          are used for the calculation of the fluctuating forces and the numerical integration. \
                          Else, the symmetric difference quotient and Simpson rule are used. Default: true");
        darboux_sum = cmdtool.get_bool("darboux_sum", true);
		cmdtool.add_usage("stationary: Boolean. If true, treats the process as stationary. Default: false.");
		stationary = cmdtool.get_bool("stationary", false);
		accelerate_stationary_decomp = cmdtool.get_bool("accelerate_stationary_decomp", false);
		cmdtool.add_usage("accelerate_stationary_decomp: Boolean. If true, uses FFT to compute the spectral decomposition \
                     and to draw the fluctuating forces with optimal run-time complexity. \
                    No effect if <stationary> is false or if <gaussian_init_val> is true. Default: false.");

	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	cout << "PARAMETERS: " << endl;
	cout << "num_sim" << '\t'<< num_sim << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "shift" << '\t'<< shift << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;
	cout << "gaussian_init_val" << '\t'<< gaussian_init_val << endl;
	cout << "darboux_sum" << '\t' << darboux_sum << endl;
	cout << "stationary" << '\t' << stationary << endl;
	cout << "accelerate_stationary_decomp" << '\t' << accelerate_stationary_decomp << endl;

	filesystem::path out_path = out_folder;

    if(!stationary)
    {
        tensor<double,4> kernel;
        cout << "Load memory kernel from: " << out_path/"kernel.f64" << endl;
        kernel.read(out_path/"kernel.f64");

        tensor<double,3> drift;
        cout << "Load drift from: " << out_path/"drift.f64" << endl;
        drift.read(out_path/"drift.f64");

        tensor<double,2> ff_average;
        cout << "Load ff_average from: " << out_path/"ff_average.f64" << endl;
        ff_average.read(out_path/"ff_average.f64");

        tensor<double,3> trajectories;
        cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
        trajectories.read(out_path/"traj.f64");

        tensor<double,1> times;
        cout << "Load times from: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");

        tensor<double,1> mean_initial_value;
        if(shift)
        {
            cout << "Load mean initial value from: " << out_path/"mean_initial_value.f64" << endl;
            mean_initial_value.read(out_path/"mean_initial_value.f64");
        }

        cout << "Initialize random force generator." << endl;
        RandomForceGenerator rfg;
        try
        {
            tensor<double,4> ff_decomp;
            cout << "Load rotation matrix from: " << out_path/"ff_decomp.f64" << endl;
            ff_decomp.read(out_path/"ff_decomp.f64");

            rfg.init_decomp(ff_average,ff_decomp);
            ff_average.clear();
            ff_decomp.clear();
        }
        catch(exception &ex)
        {
            cout << "Unable to load rotation matrix." << endl;

            tensor<double,4> ff_cov;
            cout << "Load covariance matrix of fluctuating forces from: " << out_path/"ff_cov.f64" << endl;
            ff_cov.read(out_path/"ff_cov.f64");

            rfg.init_cov(ff_average,ff_cov,out_path);
            ff_average.clear();
            ff_cov.clear();
        }

        cout << "Draw fluctuating forces and simulate trajectories." << endl;
        tensor<double,3> sim = KernelMethods::simulateTrajectories(
            trajectories,
            drift,
            kernel,
            mean_initial_value,
            rfg,
            times[1]-times[0],
            shift,
            gaussian_init_val,
            darboux_sum,
            num_sim,
            out_path);
        cout << "Write simulated trajectories: " << (out_path/"SIM")/"traj.f64" << endl;
        sim.write("traj.f64",out_path/"SIM");
        cout << "Write times: " << (out_path/"SIM")/"times.f64" << endl;
        times.write("times.f64",out_path/"SIM");
        if(txt_out)
        {
            tensor<double,2> sim_traj({sim.shape[1],sim.shape[2]});
            for(size_t n=0;n<num_sim;n++)
            {
                sim_traj << sim(n);
                InputOutput::write(times,sim_traj,(out_path/"SIM")/("sim_"+to_string(n)+".txt"));
            }
        }
    }
    else
    {
        tensor<double,3> kernel;
        cout << "Load memory kernel from: " << out_path/"kernel_stationary.f64" << endl;
        kernel.read(out_path/"kernel_stationary.f64");

        tensor<double,2> drift;
        cout << "Load drift from: " << out_path/"drift_stationary.f64" << endl;
        drift.read(out_path/"drift_stationary.f64");

        tensor<double,2> ff_average;
        cout << "Load ff_average from: " << out_path/"ff_average.f64" << endl;
        ff_average.read(out_path/"ff_average.f64");

        tensor<double,3> trajectories;
        cout << "Load trajectories from: " << out_path/"traj.f64" << endl;
        trajectories.read(out_path/"traj.f64");

        tensor<double,1> times;
        cout << "Load times from: " << out_path/"times.f64" << endl;
        times.read(out_path/"times.f64");

        tensor<double,1> mean_initial_value;
        if(shift)
        {
            cout << "Load mean initial value from: " << out_path/"mean_initial_value.f64" << endl;
            mean_initial_value.read(out_path/"mean_initial_value.f64");
        }

        cout << "Initialize random force generator." << endl;
        RandomForceGenerator rfg;
        try
        {
            tensor<double,4> ff_decomp;
            cout << "Load rotation matrix from: " << out_path/"ff_decomp.f64" << endl;
            ff_decomp.read(out_path/"ff_decomp.f64");

            rfg.init_decomp(ff_average,ff_decomp);
            ff_average.clear();
            ff_decomp.clear();
        }
        catch(exception &ex)
        {
            cout << "Unable to load rotation matrix." << endl;

            tensor<double,3> ff_cov_stationary;
            cout << "Load covariance matrix of fluctuating forces from: " << out_path/"ff_cov_stationary.f64" << endl;
            ff_cov_stationary.read(out_path/"ff_cov_stationary.f64");
            size_t num_ts = (ff_cov_stationary.shape[0]+1)/2;
            size_t num_obs = ff_cov_stationary.shape[1];

            if(!gaussian_init_val)
            {
                if(!accelerate_stationary_decomp)
                {
                    tensor<double,4> ff_cov({num_ts,num_obs,num_ts,num_obs});
                    for(size_t t=0; t<num_ts; t++)
                    {
                        for(size_t s=0; s<num_ts; s++)
                        {
                            for(size_t i=0; i<num_obs; i++)
                            {
                                for(size_t j=0; j<num_obs; j++)
                                {
                                    ff_cov(t,i,s,j) = ff_cov_stationary(num_ts-1+t-s,i,j);
                                }
                            }
                        }
                    }
                    rfg.init_cov(ff_average,ff_cov,out_path);
                    ff_cov.clear();
                }
                else
                {
                    rfg.init_cov(ff_average,ff_cov_stationary,out_path);
                    ff_cov_stationary.clear();
                }
            }
            else
            {
                tensor<double,3> ff_cov_extended;
                cout << "Load covariance between initial values and fluctuating forces: " << out_path/"ff_cov_extended.f64" << endl;
                ff_cov_extended.read(out_path/"ff_cov_extended.f64");

                tensor<double,4> ff_cov({num_ts+1,num_obs,num_ts+1,num_obs});
                for(size_t t=0; t<num_ts; t++)
                {
                    for(size_t s=0; s<num_ts; s++)
                    {
                        for(size_t i=0; i<num_obs; i++)
                        {
                            for(size_t j=0; j<num_obs; j++)
                            {
                                ff_cov(t+1,i,s+1,j) = ff_cov_stationary(num_ts-1+t-s,i,j);
                            }
                        }
                    }
                }
                for(size_t t=0; t<=num_ts; t++)
                {
                    for(size_t i=0; i<num_obs; i++)
                    {
                        for(size_t j=0;j<num_obs;j++)
                        {
                            ff_cov(0,j,t,i) = ff_cov(t,i,0,j) = ff_cov_extended(t,i,j);
                        }
                    }
                }
                ff_cov_extended.clear();

                rfg.init_cov(ff_average,ff_cov,out_path);
                ff_cov.clear();
            }
            ff_average.clear();
        }

        cout << "Draw fluctuating forces and simulate trajectories." << endl;
        tensor<double,3> sim = KernelMethods::simulateTrajectories(
            trajectories,
            drift,
            kernel,
            mean_initial_value,
            rfg,
            times[1]-times[0],
            shift,
            gaussian_init_val,
            darboux_sum,
            num_sim,
            out_path,
            accelerate_stationary_decomp);
        cout << "Write simulated trajectories: " << (out_path/"SIM")/"traj.f64" << endl;
        sim.write("traj.f64",out_path/"SIM");
        cout << "Write times: " << (out_path/"SIM")/"times.f64" << endl;
        times.write("times.f64",out_path/"SIM");
        if(txt_out)
        {
            tensor<double,2> sim_traj({sim.shape[1],sim.shape[2]});
            for(size_t n=0;n<num_sim;n++)
            {
                sim_traj << sim(n);
                InputOutput::write(times,sim_traj,(out_path/"SIM")/("sim_"+to_string(n)+".txt"));
            }
        }
    }

    cout << "END: main_simulator" << endl << endl;
	return 0;
}
