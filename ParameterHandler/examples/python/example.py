# SPDX-FileCopyrightText: 2021 Andreas Haertel <http://andreashaertel.anno1982.de/>
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-PackageName: parameter_handler
# SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler

# Part of the parameter_handler project
# Example, demonstrating the use of parameter_handler in python

# for compatibility with Python 2 and 3
from __future__ import division, unicode_literals, print_function
# Add the location of our parameter_handler to the system path list
import sys
sys.path.append('../../bin/python')
# Import the parameter_handler
import parameter_handler as ph

# Hint for the user
print(" Please call: \"python example.py params.txt -Psize=5.4\" (see README.md)")

# Define usage of the program and parameters
# With the command "set_param_usage" descriptions of parameters can be given. 
# Giving this description is optional and the fields can contain any text; 
# we demonstrate this fact with the 5th call of the command below. 
ph.set_param_usage("mode=<int>", "Some integer specifying some mode of the program. \nThis value is required! ")
ph.set_param_usage("stiff=False/True", "If True, the watched object is stiff, otherwise not. \nStandard value is False.")
ph.set_param_usage("size=<float>", "Some size, standard value is 0.0. ")
ph.set_param_usage("some_name=<text>", "Some name of, e.g., a movie file. ")
ph.set_param_usage("As you will see", "these fields can contain any text.")

# Set additional information in a usage text. 
usage_text = "This usage-text will be shown in addition to other information. \nWe will add a few more lines.\nThis is the last line of the usage-text information."
# Read the parameter file
ph.read_file(additional_usage_text=usage_text)


# Get values of parameters. 
# Possible types are int,float,bool,str. 
# Parameters can have standard values that are assigned if the parameter is not specified. 
# If no standard is specified, the parameter is required. 
mode = ph.get_param("mode", int)
stiff = ph.get_param("stiff", bool, False)
size = ph.get_param("size", float, 0.0)
some_name = ph.get_param("some_name", str, "bar")


# Inform the user
print(" Mode set to: {0}".format(mode))
print(" stiff: {0}".format(stiff))
print(" size: {0}".format(size))
print(" some name: {0}".format(some_name))




