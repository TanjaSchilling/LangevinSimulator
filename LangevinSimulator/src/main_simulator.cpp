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
#include <gsl/gsl_blas.h>

using namespace std;
using namespace TensorUtils;

int main(int argc, char *argv[]) {

    cout << "BEGIN: main_simulator" << endl;

	size_t num_sim;
	string out_folder;
	bool shift;
	bool txt_out;
	bool gaussian_init_val;
	bool darboux_sum;
	bool stationary;

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
	} catch (const ParameterHandler::BadParamException &ex) {
		cmdtool.show_usage();
		throw ex;
	}

	if(stationary)
	{
        cout << "Set 'stationary' to false. Missing implementation." << endl;
        return 1;
	}

	cout << "PARAMETERS: " << endl;
	cout << "num_sim" << '\t'<< num_sim << endl;
	cout << "out_folder" << '\t'<< out_folder << endl;
	cout << "shift" << '\t'<< shift << endl;
	cout << "txt_out" << '\t'<< txt_out << endl;
	cout << "gaussian_init_val" << '\t'<< gaussian_init_val << endl;
	cout << "darboux_sum" << '\t' << darboux_sum << endl;

	filesystem::path out_path = out_folder;

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

    // set args
    double dt = times[1]-times[0];
    size_t num_traj = trajectories.shape[0];
    size_t num_ts = trajectories.shape[1];
    size_t num_obs = trajectories.shape[2];
    cout << "Loaded "<< num_traj << " trajectories with " << num_ts << " timesteps and " << num_obs << " observables." << endl;

    if(gaussian_init_val)
    {
        trajectories.clear();
    }

    tensor<double,3> sim;
    tensor<double,3> rand_ff({num_sim,num_ts,num_obs});
    if(darboux_sum)
    {
        cout << "Draw fluctuating forces." << endl;
        kernel=kernel.transpose({0,1,3,2});
        tensor<double,2> rand_ff_buffer;
        filesystem::create_directories(out_path/"SIM");
        sim.alloc({num_ts,num_obs,num_sim});
        for(size_t n=0;n<num_sim; n++)
        {
            rand_ff_buffer = rfg.pull_multivariate_gaussian();
            if(!gaussian_init_val)
            {
                for(size_t i=0; i<num_obs; i++)
                {
                    sim(0,i,n) = trajectories(n%num_traj,0,i); // set initial value
                }
                memcpy(&rand_ff(n),&rand_ff_buffer[0],num_ts*num_obs*sizeof(double));
            }
            else
            {
                for(size_t i=0; i<num_obs; i++)
                {
                    sim(0,i,n) = rand_ff_buffer(0,i); // set initial value
                }
                memcpy(&rand_ff(n),&rand_ff_buffer(1),num_ts*num_obs*sizeof(double));
            }
        }
        cout << "Simulate trajectories." << endl;
        gsl_matrix * out = gsl_matrix_alloc(num_obs,num_sim);
        tensor<double,2> buff({num_obs,num_sim},0.0);
        for(size_t t=0;t+1<num_ts;t++)
        {
            if(t>0)
            {
                gsl_matrix_const_view kernel_t1 = gsl_matrix_const_view_array(&kernel(t),t*num_obs,num_obs);
                gsl_matrix_const_view trajectory = gsl_matrix_const_view_array(&sim[0],t*num_obs,num_sim);
                gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&kernel_t1.matrix,&trajectory.matrix,0.0,out);
                buff << *out->data;
            }
            buff*=dt;
            for(size_t n=0;n<num_sim;n++)
            {
                for(size_t i = 0; i<num_obs; i++)
                {
                    for(size_t k = 0; k<num_obs; k++)
                    {
                        buff(i,n) += drift(t,i,k)*sim(t,k,n);
                    }
                    buff(i,n) += rand_ff(n,t,i);
                }
                for(size_t i=0;i<num_obs;i++)
                {
                    sim(t+1,i,n) = sim(t,i,n)+dt*buff(i,n);
                }
            }
        }
        gsl_matrix_free(out);
        if(shift)
        {
            for(size_t n=0;n<num_sim; n++)
            {
                for(size_t t=0; t<num_ts; t++)
                {
                    for(size_t o=0; o<num_obs; o++)
                    {
                        sim(t,o,n) += mean_initial_value[o];
                    }
                }
            }
        }
        sim=sim.transpose({2,0,1});
    }
    else
    {
        cout << "Simulate trajectories." << endl;
        tensor<double,2> rand_ff_buffer;
        tensor<double,2> rand_ff_n({num_ts,num_obs});
        tensor<double,2> simulated_trajectory({num_ts,num_obs});
        RK4 rk4;
        filesystem::create_directories(out_path/"SIM");
        sim.alloc({num_sim,num_ts,num_obs});
        for(size_t n=0;n<num_sim; n++)
        {

            rand_ff_buffer = rfg.pull_multivariate_gaussian();

            if(!gaussian_init_val)
            {
                for(size_t i=0; i<num_obs; i++)
                {
                    simulated_trajectory(0,i) = trajectories(n%num_traj,0,i); // set initial value
                }
                rand_ff_n = rand_ff_buffer;
            }
            else
            {
                for(size_t i=0; i<num_obs; i++)
                {
                    simulated_trajectory(0,i) = rand_ff_buffer(0,i); // set initial value
                }
                rand_ff_n << rand_ff_buffer(1);
            }

            rk4.integrate(dt,drift,kernel,simulated_trajectory,rand_ff_n);

            if(shift)
            {
                for(size_t t=0; t<num_ts; t++)
                {
                    for(size_t o=0; o<num_obs; o++)
                    {
                        simulated_trajectory(t,o) += mean_initial_value[o];
                    }
                }
            }
            simulated_trajectory >> sim(n);
            rand_ff_n >> rand_ff(n);
        }
    }
    cout << "Write random fluctuating forces: "<< endl;
    rand_ff.write("rand_ff.f64",out_path/"SIM");
    cout << "Write simulated trajectories: " << (out_path/"SIM")/"traj.f64" << endl;
    sim.write("traj.f64",out_path/"SIM");
    cout << "Write times: " << (out_path/"SIM")/"times.f64" << endl;
    times.write("times.f64",out_path/"SIM");
    if(txt_out)
    {
        tensor<double,2> sim_traj({num_ts,num_obs});
        for(size_t n=0;n<num_sim;n++)
        {
            sim_traj << sim(n);
            InputOutput::write(times,sim_traj,(out_path/"SIM")/("sim_"+to_string(n)+".txt"));
        }
    }
    cout << "END: main_simulator" << endl << endl;


	return 0;
}
