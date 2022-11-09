// SPDX-FileCopyrightText: 2021 Andreas Haertel <http://andreashaertel.anno1982.de/> // NOLINT
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-PackageName: parameter_handler
// SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler

//
// A minimal example for the usage of the parameter handler.
//

#include "../../include/parameter_handler.hpp"



int main(int argc, char **args) {
  // Inform user:
  std::cout << " Please call: " << std::endl
            << " > " << args[0] << " -f some_params.txt -p size=5.4"
            << " (see README.md)"
            << std::endl;

  // Initialise the ParameterHandler by passing through all command line
  // arguments to the constructor.
  ParameterHandler ph(argc, args);

  // We do not set any usage information and do not process the "-h","--help"
  // and "-e","--export" flags.

  // Just read in some parameters from the file some_params.txt and
  // from the command line:
  //
  // Handle the flags "f","file" and "p","param":
  // All parameters are collected from an optional parameter file
  // (flag "f","file") and from the command line (flag "p","param").
  // If this function is not called, no parameter is available by the
  // different get_xxx functions.
  ph.process_parameters();

  // Get values of parameters.
  // Possible types are int,double,bool,char*,std::string.
  // Get the required parameter "mode".
  // Catch BadParamException, if "mode" is not defined.
  int mode = 0;
  try {
    mode = ph.get_int("mode");
  } catch (std::exception *e) {
    std::cout << " Exception message: " << e->what() << std::endl;
    // Typically, you would stop the code here.
  }
  // Get the optional parameter "stiff" with standard value "false"
  bool stiff = ph.get_bool("stiff", false);
  // Get the optional parameter "size" with standard value "0.0"
  double size = ph.get_double("size", 0.0);
  // Get the optional parameter "some_name" with standard value "bar"
  std::string some_name = ph.get_string("some_name", "bar");
  // Inform user
  std::cout << " Mode set to: " << mode << std::endl;
  std::cout << " stiff: " << stiff << std::endl;
  std::cout << " size: " << size << std::endl;
  std::cout << " some name: " << some_name << std::endl;


  // Exit the program with return code 0
  return 0;
}







