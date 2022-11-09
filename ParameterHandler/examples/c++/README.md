<!--
SPDX-FileCopyrightText: 2021 Andreas Härtel <http://andreashaertel.anno1982.de/>
SPDX-License-Identifier: CC-BY-SA-4.0
SPDX-PackageName: parameter_handler
SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler
-->

# C++ examples

To run the examples, you have first to compile the library and, afterwards, 
run the following commands in the examples/c++ directory: 
```bash
make init
make
```

## A minimal example

The minimal example shows how to read parameters from the file some_params.txt and 
from the command line. As a starting point, 
run the following commands in the examples/c++ directory: 
```bash
./bin/${HOST}.minimal_example -f some_params.txt -p size=5.4
```
Checking the source file src/minimal_example.cpp and the parameter file 
some_params.txt should give a first explanation on how the parameter_handler works. 





## A full example

The ParameterHandler class can do much more than shown in the minimal example. 
If you go through the source code in src/example.cpp and 
run the following commands in the examples/c++ directory and follow the 
hints given in the output, you should get an advanced overview over the 
ParameterHandler: 
```bash
./bin/${HOST}.example -h
./bin/${HOST}.example 
./bin/${HOST}.example -p mode=1 -e params.log
./bin/${HOST}.example -f params.log
./bin/${HOST}.example -p mode=2 -f params.log -p mode=3 foo -p some_thing=True
./bin/${HOST}.example -f params.log CONTINUE -F apple 9 90 -F cherry one

```





