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

from ParameterHandler import ParameterHandler, run
import os

###############################################################################################################
# COMPUTE CORRELATION FUNCTION, MEMORY KERNEL, DRIFT TERM, FLUCTUATING FORCES AND SIMULATE TRAJECTORIES
###############################################################################################################

# set parameters (see 'ParameterHandler.py' for details)
params = ParameterHandler()
params.out_folder = "./TEST_OUT_2"
params.in_folder = "./TEST_DATA_2"
params.shift = True 
params.fluctuating_force = True 
params.num_sim = 20000
params.txt_out = False
params.mollifier_width = 5
params.gaussian_init_val = False
params.darboux_sum = True
params.stationary = True

# RUN CPP EXECUTABLES
run(params)

# save this script
os.system("cp "+os.path.basename(__file__)+" "+params.out_folder+'/'+"run.py")

###############################################################################################################
# EVALUATE SIMULATIONS
###############################################################################################################

params.out_folder = "./TEST_OUT_2_SIM"
params.in_folder = "./TEST_OUT_2/SIM"
params.fluctuating_force = False 
params.num_sim = 0
params.txt_out = False
params.mollifier_width = 0

# RUN CPP EXECUTABLES
run(params)

# save this script
os.system("cp "+os.path.basename(__file__)+" "+params.out_folder+'/'+"run.py")


