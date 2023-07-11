"""
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
"""

import os

def run(param_handler):
    # normalize paths and create directories
    param_handler.out_folder=os.path.normpath(param_handler.out_folder)
    param_handler.in_folder=os.path.normpath(param_handler.in_folder)
    os.system("mkdir -p "+param_handler.out_folder)
    
    # write parameter file for ccp executables
    parameter_filename = param_handler.out_folder+'/parameter.txt'
    param_handler.write_parameter_file(parameter_filename)
    
    # RUN CPP EXECUTABLES
    os.system(param_handler.corr_exe+" -f "+parameter_filename+" | tee "+param_handler.out_folder+"/corr_log.txt") 		# cross-correlation matrix
    os.system(param_handler.kernel_exe+" -f "+parameter_filename+" | tee "+param_handler.out_folder+"/kernel_log.txt")		# memory kernel
    if param_handler.fluctuating_force:
        os.system(param_handler.ff_exe+" -f "+parameter_filename+" | tee "+param_handler.out_folder+"/ff_log.txt")		# fluctuating forces
    if param_handler.num_sim > 0:
        os.system(param_handler.sim_exe+" -f "+parameter_filename+" | tee "+param_handler.out_folder+"/sim_log.txt")		# simulate trajectories

class ParameterHandler:
    def __init__(self,
                 out_folder="./OUT",
                 in_folder="./TEST_DATA",
                 in_prefix="",
                 file_range="0-999",
                 num_obs=1,
                 t_min=0.0,
                 t_max=1000.0,
                 increment=1,
                 shift=True,
                 fluctuating_force=False,
                 num_sim=0,
                 txt_out=True,
                 corr_exe="./bin/Release/main_correlation",
                 kernel_exe="./bin/Release/main_kernel",
                 ff_exe="./bin/Release/main_fluctuating_forces",
                 sim_exe="./bin/Release/main_simulator",
                 mollifier_width=10,
                 gaussian_init_val=False,
                 darboux_sum=True,
                 stationary=False,
                 chop_stationary_trajectories=0):
        self.out_folder = out_folder
        self.in_folder = in_folder
        self.in_prefix = in_prefix
        self.file_range = file_range
        self.num_obs = num_obs
        self.t_min = t_min
        self.t_max = t_max
        self.increment = increment
        self.shift = shift
        self.fluctuating_force = fluctuating_force
        self.num_sim = num_sim
        self.txt_out = txt_out
        self.corr_exe = corr_exe
        self.kernel_exe = kernel_exe
        self.ff_exe = ff_exe
        self.sim_exe = sim_exe
        self.mollifier_width = mollifier_width
        self.gaussian_init_val = gaussian_init_val
        self.darboux_sum = darboux_sum
        self.stationary = stationary
        self.chop_stationary_trajectories = chop_stationary_trajectories

    def get_parameter_file(self):
        parameter_file = """\
out_folder {out_folder}
in_folder {in_folder}
in_prefix {in_prefix}
file_range {file_range}
num_obs {num_obs}
t_min {t_min}
t_max {t_max}
increment {increment}
shift {shift}
num_sim {num_sim}
txt_out {txt_out}
mollifier_width {mollifier_width}
gaussian_init_val {gaussian_init_val}
darboux_sum {darboux_sum}
stationary {stationary}
chop_stationary_trajectories {chop_stationary_trajectories}""".format(
            out_folder=self.out_folder,
            in_folder=self.in_folder,
            in_prefix=self.in_prefix,
            file_range=self.file_range,
            num_obs=self.num_obs,
            t_min=self.t_min,
            t_max=self.t_max,
            increment=self.increment,
            shift=self.shift,
            num_sim=self.num_sim,
            txt_out=self.txt_out,
            mollifier_width=self.mollifier_width,
            gaussian_init_val=self.gaussian_init_val,
            darboux_sum=self.darboux_sum,
            stationary=self.stationary,
            chop_stationary_trajectories=self.chop_stationary_trajectories)
        return parameter_file

    def write_parameter_file(self, parameter_filename):
        out_file = open(parameter_filename, 'w')
        out_file.write(self.get_parameter_file())
        out_file.close()
