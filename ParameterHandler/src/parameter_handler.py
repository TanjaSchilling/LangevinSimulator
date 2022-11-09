# SPDX-FileCopyrightText: 2018-2021 Andreas Haertel <http://andreashaertel.anno1982.de/>
# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-PackageName: parameter_handler
# SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler
################################################################################################
#
# parameter_handler.py
#
# -- Handle parameters from a file or command line in python --
# 
# void set_param_usage( parameter , usage )
# void print_usage( (optional)disable_cmd , (optional)additional_usage_text )
# void read_file( (optional)filename , (optional)disable_cmd , (optional)additional_usage_text )
# get_param( specifier, data_type, (optional)std_value )
#
################################################################################################


import sys




######################################
######################################
# Global parameters

_GLOB_param_data = []
_GLOB_param_info = []




######################################
######################################
#
# void set_param_usage( parameter , usage )
#
# Defines the usage of a certain parameter that is shown in the 
# usage information of the program, i.e., if --help is called. 
# + parameter
#   defines the parameter name
# + usage
#   is a text describing the parameter. 
# This should be called before calling print_usage and before calling read_file. 
#
def set_param_usage( parameter , usage ): 
	data = []
	data.append(parameter)
	data.append(usage)
	_GLOB_param_info.append(data)




######################################
######################################
#
# void print_usage( (optional)disable_cmd , (optional)additional_usage_text )
#
# Print the usage of this param_file_reader. 
# If disable_cmd==True, the corresponding information is not shown. 
# The usage of parameters is shown, if it has been set. 
# If additional_usage_text is set, the text will be add to the output. 
#
def print_usage( disable_cmd=False , additional_usage_text=None ): 
	print(" Usage: python %s <parameter file>" % (sys.argv[0]))
	print(" System parameters can be defined in the parameter file, one per line: ")
	print("   <parameter1> <value>")
	print("   <parameter2> <value>")
	print("   ... ")
	print(" For comments, # can be used: everything in a line after # will be ignored. ")
	if not disable_cmd: 
		print(" Parameters can also be specified in the command line by using -P<param>=<value>. ")
		print(" Command line parameters overwrite parameters from the parameter file. ")
	# show parameter information, if set
	no_params = len(_GLOB_param_info)
	if no_params > 0: 
		print(" Parameter specification: ")
		for i in range(no_params): 
			data = _GLOB_param_info[i]
			print(" + {0} ".format(data[0]))
			info = (data[1]).replace("\n", "\n   ")
			print("   {0}".format(info))
	# print additional_usage if available
	if not (additional_usage_text == None): 
		print(additional_usage_text)




######################################
######################################
#
# void read_file( (optional)filename , (optional)disable_cmd , (optional)additional_usage_text )
#
# Read in parameter file specified by filename. 
# If filename is not specified, the filename is assumed to by set as the 
# first command line argument. 
# In general, parameters can also be specified in the command line by using -P<param>=<value>. 
# Command line parameters overwrite parameters from the parameter file, because they are stored 
# in the _GLOB_param_data first. 
# If disable_cmd=True is set, parameters from the command line will be ignored. 
# The text in additional_usage_text will be add to the print_usage output. 
#
def read_file( filename=None , disable_cmd=False , additional_usage_text=None , verbose=True ): 
	# if filename is not specified, read it from the command line
	if filename == None: 
		# Get filename from command line
		if len(sys.argv) < 2:
			print(" Error: no parameter file indicated. ")
			print_usage(disable_cmd, additional_usage_text)
			sys.exit(0)
		if sys.argv[1] == "--help": 
			print_usage(disable_cmd, additional_usage_text)
			sys.exit(0)
		filename = sys.argv[1]
		if verbose: 
		  print("# Parameter file specified: %s" % (filename))
	
	# open the parameter file
	try: 
		param_file = open(filename)
	#except KeyboardInterrupt: 
	#	print "Shutdown requested...exiting"
	except Exception: 
		#traceback.print_exc(file=sys.stdout)
		print(" Error: parameter file not found or cannot be opened. ")
		sys.exit(0)

	# read parameters from the command line
	# and store them in _GLOB_param_data
	if disable_cmd == False: 
		# search for parameters in the command line
		for i in range(len(sys.argv)-2): 
			line = sys.argv[i+2]
			if not line.startswith('-P'):
				continue
			line = line[2:]
			data = line.split('=')
			# ignore wrong or incomplete fields
			if len(data) != 2: 
				continue
			# store data
			if verbose: 
			  print("# Parameter red from command line: {0}={1}".format(data[0],data[1]))
			_GLOB_param_data.append(data)

	# read the parameter file into _GLOB_param_data
	try: 
		for line in param_file: 
			# ignore comments
			if line.startswith('#'): 
				continue
			data = line.split()
			# ignore empty lines or incomplete fields
			if len(data) != 2: 
				continue
			# store data
			_GLOB_param_data.append(data)
		# close file
		param_file.close()
	except Exception: 
		#traceback.print_exc(file=sys.stdout)
		print(" Error: cannot read from parameter file. ")
		sys.exit(0)
	



######################################
######################################
#
# get_param( specifier, data_type, (optional)std_value )
#
# Function for getting a parameter from the parameter file. 
#  specifier: String to specify the name of the parameter in the file. 
#  data_type: Type of the parameter, e.g., int, float, str, bool. 
#  std_value: If the specified parameter is not found, this value is returned. 
#             If it is not set, an error is thrown. 
# 
def get_param( specifier, data_type, std_value=None ): 
	# search for specified data
	found = -1
	for i in range(len(_GLOB_param_data)):
		if _GLOB_param_data[i][0] == specifier: 
			found = i
			break
	if found < 0: 
		# specifier not found: exit or return standard value
		if std_value == None: 
			print(" Error: parameter %s has not been specified in the parameter file. " % (specifier))
			sys.exit(0)
		return std_value;
	# cast the corresponding value
	if data_type == int: 
		try:
			value = int(_GLOB_param_data[i][1])
		except Exception: 
			print(" Error: wrong data type for parameter %s found. " % (specifier))
			sys.exit(0)
	elif data_type == str: 
		try:
			value = str(_GLOB_param_data[i][1])
		except Exception: 
			print(" Error: wrong data type for parameter %s found. " % (specifier))
			sys.exit(0)
	elif data_type == float: 
		try:
			value = float(_GLOB_param_data[i][1])
		except Exception: 
			print(" Error: wrong data type for parameter %s found. " % (specifier))
			sys.exit(0)
	elif data_type == bool: 
		try:
			#value = bool(_GLOB_param_data[i][1])
			value = False
			if _GLOB_param_data[i][1] in ['true', 'True', 'TRUE', '1']: 
				value = True
		except Exception: 
			print(" Error: wrong data type for parameter %s found. " % (specifier))
			sys.exit(0)
	else: 
		# throw error
		print(" Error: unknown data type requested. ")
		sys.exit(0)
	return value;









######################################
######################################

if __name__ == "__main__": 
	print(" Read the parameter filename from the command line. ")
	read_file()
	print(" Try to read the parameter TEST as a float with standard value 3.151. ")
	print(" Value red from file: %s" % (get_param("TEST", float, 3.151)))








