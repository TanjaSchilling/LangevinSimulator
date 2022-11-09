# LangevinSimulator

LangevinSimulator is licensed, open-source software. 
LangevinSimulator comes with verbatim copies of the software "parameter_handler" as well as "TensorUtils", 
which is licensed, open-source software that is NOT part of LangevinSimulator. 
Please see sec. **Licenses** at the end of this document for details.


## Cite us

Please cite our work in scientific publications. 
Citation entries as `RIS` and `BibTeX` are given in `MyParentFolder/LangevinSimulator/citation`.
The software LangevinSimulator is based on the following articles:
- 2022: C. Widder, F. Glatzel & T. Schilling

	Generalized Langevin dynamics simulation with non-stationary memory kernels: How to make noise  ([article]() [preprint](https://arxiv.org/abs/2209.11021))



## Introduction

LangevinSimulator offers methods to compute a data-driven coarse-grained model of Hamiltonian systems out of equilibrium. 
Formally, the non-stationary, generalized Langevin equation (nsGLE) is derived within a Mori-type projection operator formalism.
The nsGLE is the equation of motion for the coarse-grained observable, which is a function of phase space.
LangevinSimulator is capable of simulating new, self-consistent trajectories by integrating the equations of motion.

Based on a given set of original trajectories, LangevinSimulator allows to extract the so called memory kernel, that is needed to integrate the equations of motion. Further, the drift term and fluctuating forces are needed to simulate new trajectories. 
While the memory kernel and drift term are ensemble-averaged quantities, the fluctuating forces are themselves trajectories depending on the initial state in phase space. LangevinSimulator offers routines to compute the fluctuating forces for each trajectory as well as the drift term. From the fluctuating forces corresponding to each original trajectory, their mean value and auto-correlation function is estimated. As proven in the article "Generalized Langevin dynamics simulation with non-stationary memory kernels: How to make noise", this is enough to simulate self-consistent trajectories that reproduce the mean and auto-correlation function and consequently reproduce the drift term as well as the memory kernel. LangevinSimulator implements this technique by drawing the fluctuating forces from a multivariate Gaussian distribution with proper mean and covariance matrix. Before the numerical evaluation, the mean initial value is subtracted from the trajectories. After the numerical integration, the mean initial value is added onto the simulations. This way, correlations between the fluctuating forces and initial values are taken into account.

The full documentation is located at `MyParentFolder/LangevinSimulator/doc/`.


## Build instructions

### Compile with Codeblocks

Open and compile the Release/Debug targets of the following CodeBlocks projects in `MyParentFolder/LangevinSimulator`:
- `main_correlation.cbp`
- `main_kernel.cbp`
- `main_fluctuating_forces.cbp`
- `main_simulator.cbp`


### Compile with make

To compile all executables open a terminal in the folder `MyParentFolder/LangevinSimulator` and type:
```
	./make_all.sh
```	
This will `make` the following makefiles at `MyParentFolder/LangevinSimulator`:
- `makefile_correlation`
- `makefile_kernel`
- `makefile_fluctuating_forces`
- `makefile_simulator`

The binaries are located in `LangevinSimulator/bin/Release` and `LangevinSimulator/bin/Debug`. 


### Customize your build

If you desire to customize your build options, it is recommended to open the CodeBlocks projects and modify the build settings within CodeBlocks.
After the project is saved you may use `cbp2make` in order to generate your desired makefile, e.g.
```
	sudo apt install cbp2make
	cd MyParentFolder/LangevinSimulator
	cbp2make -in main_correlation.cbp -out makefile_correlation
```
You are now ready to compile using `make`, e.g.
```
	make -f makefile_correlation
```	


## Usage

You may execute the code using **CodeBlocks**, the built-in **C++ Interface** or the **Python Interface**.
A description of all available parameters is given in sec. **Parameters**. 

Please ensure that the input trajectories are formatted properly. 
Example trajectories are located at `MyParentFolder/LangevinSimulator/TEST_DATA`.
The first column contains the times and the following columns 
contain the corresponding values for the observables.
The first few lines from the example trajectory `dipoleMoment0000.txt` are:
```
# System Size:	12.000000000000000	12.000000000000000	12.000000000000000
# Number of Particles:	125
# Timestep:	0.010000000000000
# WCA-Parameters:
# 	Epsilon:	1.000000000000000
# 	Sigma:	2.000000000000000
# Data Format:	time	mu_x	mu_y	mu_z
0.000000000000000	125.000000000000000	0.000000000000000	0.000000000000000
0.100000000000000	124.999746318223842	0.000039394462241	-0.000022892471865
0.200000000000000	124.998984698373746	0.000156246425438	-0.000088155065376
0.300000000000000	124.997713835234592	0.000348563439825	-0.000180648094650
0.400000000000000	124.995931782722295	0.000626572571891	-0.000293840621097
0.500000000000000	124.993636080296639	0.000990302342154	-0.000415916202714
0.600000000000000	124.990823678351745	0.001436710617749	-0.000538403967656
0.700000000000000	124.987490854766818	0.001965428796087	-0.000653031331692
0.800000000000000	124.983633337966822	0.002578609901494	-0.000755257410848
0.900000000000000	124.979246078958454	0.003275395739426	-0.000836776042212
```


The following subsections explain how to calculate the following quantities:
1) correlation matrix
2) memory kernel
3) fluctuating forces (and drift term) 
4) (self-consistent) simulations

Please mind that these quantities must be computed in consecutive order. 


### CodeBlocks

Open and run the CodeBlocks projects at `MyParentFolder/LangevinSimulator`:
1) `main_correlation.cbp`
2) `main_kernel.cbp`
3) `main_fluctuating_forces.cbp`
4) `main_simulator.cbp`

The parameters are set in `MyParentFolder/LangevinSimulator/parameter.txt`, 
see next subsection for details.

### C++ Interface 

The C++ Interface uses the software "parameter_handler" to read parameters from a text file. 
A verbatim copy of "parameter_handler" is located in `MyParentFolder/ParameterHandlder`.
The parameter file for the given test data in `MyParentFolder/LangevinSimulator/TEST_DATA` could look like
```
out_folder TEST_OUT
in_folder TEST_DATA
in_prefix dipoleMoment
file_range 0-99
num_obs 2
t_min 50
t_max 90
increment 5
shift 1
num_sim 1000
txt_out True
```
This parameter file is stored at `MyParentFolder/LangevinSimulator/parameter.txt`. 

Navigate to `MyParentFolder/LangevinSimulator` and 

- calculate the correlation function
```
	./bin/Release/main_correlation -f parameter.txt
```

- calculate the memory kernel
```
	./bin/Release/main_kernel -f parameter.txt
```

- calculate the fluctuating forces for each trajectory as well as the drift term 
```
	./bin/Release/main_fluctuating_forces -f parameter.txt
```

- simulate trajectories
```
	./bin/Release/main_simulator -f parameter.txt
```


### Python Interface

The python interface consists of two python scripts `ParameterHandler.py` and `run.py` in `MyParentFolder/LangevinSimulator/`. 
You may test all cpp executables with the test data from `MyParentFolder/LangevinSimulator/TEST_DATA` as follows:
```
	cd MyParentFolder/LangevinSimulator
	python3 run.py
```
Please note that the script is supposed to be located and executed in `MyParentFolder/LangevinSimulator`.

#### `ParameterHandler.py`

The file `ParameterHandler.py` contains the actual interface, 
which is the `run(param_handler)` function that writes the parameter file and passes it to the cpp executables for you.
The variable `param_handler` is an instantiation from the class `ParameterHandler`.
It contains all available parameters as attributes. 
Please have a look at the implementation at `MyParentFolder/LangevinSimulator/ParameterHandler.py` for details.

#### `run.py`

The purpose of this script is to set all parameters which are passed to the interface on execution.
Please have a look at the following implementation:
```
from ParameterHandler import ParameterHandler, run
import os

###############################################################################################################
# COMPUTE CORRELATION FUNCTION, MEMORY KERNEL, DRIFT TERM, FLUCTUATING FORCES AND SIMULATE TRAJECTORIES
###############################################################################################################

# set parameters (see 'ParameterHandler.py' for details)
params = ParameterHandler()
params.out_folder = "./TEST_OUT"
params.in_folder = "./TEST_DATA"
params.in_prefix = "dipoleMoment"
params.file_range = "0-99"
params.num_obs = 2;
params.t_min = 50
params.t_max = 90
params.increment = 5
params.shift = True 
params.fluctuating_force = True 
params.num_sim = 1000
params.txt_out = True
params.corr_exe = "./bin/Release/main_correlation"
params.kernel_exe = "./bin/Release/main_kernel"
params.ff_exe = "./bin/Release/main_fluctuating_forces"
params.sim_exe = "./bin/Release/main_simulator"

# RUN CPP EXECUTABLES
run(params)

# save this script
os.system("cp "+os.path.basename(__file__)+" "+params.out_folder+'/'+"run.py")
```
Note that this script will save itself to `<out_folder>/<name_of_run_script>`, 
so you do not need to store your parameters yourself.
Further, it is possible to execute several scripts in parallel. 
To this end, simply create a copy `my_run.py` within `MyParentFolder/LangevinSimulator`,
set a new `<out_folder>` and proceed as usual.

### Parameters

The following parameters are used for the cpp interface as well as the python interface.

- `<out_folder>`: 
	
	Directory in which all output files are stored. Default: `"./OUT"`. Type: string.

- `<in_folder>`: 
	
	Directory where the input trajectories are located. Default: `"./TEST_DATA"`. Type: string.

- `<in_prefix>`: 
	
	Input trajectories must be stored as `<in_folder>/<in_prefix><n>.txt`, where `<n>` is a positive integer. Default `""`. Type: string.

- `<file_range>`: 
	
	Specify the input trajectories to be used. Takes the form `"0-99"` or `"0-99,200-299"`. Default: `"0-999"`. Type: string.

- `<num_obs>`: 
	
	Number of observables to be used. Default `1`. Type: positive integer.

- `<t_min>`: 
	
	Initial time. Default `0.0`. Type: floating point.

- `<t_max>`: 

	End of desired time interval. Default `1000.0`. Type: floating point.

- `<increment>`: 

	Only use every `<increment>`-th time step. Default `1`. Type: positive integer.

- `<shift>`: 
	
	Enable (recommended) or disable the shift. The shift will subtract the average initial value from your trajectories before numerical evaluation. The shift will be added again after the simulations have been carried out. Note that in general you will only obtain self-consistent simulations if `<shift>` is set to true. Default `True`. Type: boolean.

- `<num_sim>`: 
	
	Number of simulations to be carried out. Default `0`. Type: unsigned integer.

- `<text_out>`: 
	
	Enable/disable output files as text files. Default `True`. Type: boolean. 
	
The follwing parameters are only used by the python interface.

- `<fluctuating_force>`: 
	
	Enable/disable the calculation of the fluctuating forces. Default: `False`. Type: boolean.

- `<corr_exe>`:
	
	Set the path to the executable `main_correlation`. Default: `"./bin/Release/main_correlation"`. Type: string.

- `<kernel_exe>`:
	
	Set the path to the executable `main_kernel`. Default: `"./bin/Release/main_kernel"`. Type: string.

- `<ff_exe>`:
	
	Set the path to the executable `main_fluctuating_forces`. Default: `"./bin/Release/main_fluctuating_forces"`. Type: string.

- `<sim_exe>`:
	
	Set the path to the executable `main_simulator`. Default: `"./bin/Release/main_simulator"`. Type: string.

## Utilities

Please have a look at `MyParentFolder/LangevinSimulator/utilities` for some additional python utilities in order to read/write and plot your data.


## Trouble shooting

In the unfortunate case that an error occurs, please send a bug report to <BUGREPORT@EMAIL>. 
You may want to try the debug builds in `MyParentFolder/LangevinSimulator/bin/Debug`.
When using the python interface, simply set the executable paths in your `run.py` accordingly.
The debug builds support debugging symbols and additional error-handling from within TensorUtils.

## Licenses

### parameter_handler: Redistribution of licensed, open-source software.

Please find the licenses for all files from the software "parameter_handler" in its verbatim copy at `MyParentFolder/ParameterHandler` or visit <https://github.com/andreashaertel/parameter_handler>.


### TensorUtils: Redistribution of licensed, open-source software.

Please find the license for all files from the software "TensorUtils" in its verbatim copy at ``MyParentFolder/TensorUtils`` or visit <https://github.com/TensorUtils/TensorUtils>.


### LangevinSimulator: Licensed, open-source software.

Please find the license "LICENSE" for the software "LangevinSimulator" in its source directory `MyParentFolder/LangevinSimulator`.
All files within the source directory `MyParentFolder/LangevinSimulator` (and subfolders) 
are part of the software "LangevinSimulator" and are licensed under the GNU GPL3, if not stated otherwise.

**LangevinSimulator Version 1.0**

**Copyright 2020-2022 Christoph Widder, Fabian Glatzel and Tanja Schilling**

Christoph Widder <christoph.widder[at]merkur.uni-freiburg.de>
Fabian Glatzel <fabian.glatzel[at]physik.uni-freiburg.de>
Tanja Schilling <tanja.schilling[at]physik.uni-freiburg.de>

This file is part of LangevinSimulator.

LangevinSimulator is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

LangevinSimulator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LangevinSimulator.
If not, see <https://www.gnu.org/licenses/>.


