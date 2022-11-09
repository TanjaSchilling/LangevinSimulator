// SPDX-FileCopyrightText: 2021 Andreas Haertel <http://andreashaertel.anno1982.de/> // NOLINT
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-PackageName: parameter_handler
// SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler

//
// Full example for the usage of the parameter handler.
//
// Call the program using the flag --help for further information.
//

#include "../../include/parameter_handler.hpp"





int main(int argc, char **args) {
  // Inform user
  std::cout << " To get further information, call: " << std::endl
            << "   " << args[0] << " --help" << std::endl << std::endl;

  // Initialise the ParameterHandler by passing through all command line
  // arguments to the constructor.
  ParameterHandler ph(argc, args);

  // (Optional) We add information about the usage of the program.
  // We add this information line by line
  // (either std::string or char* are possible).
  ph.add_usage("This is the usage text.");
  ph.add_usage("This program will sort fruits specified by the user via their");
  ph.add_usage("name (e.g. apple) and size in cm (e.g. 5). ");
  ph.add_usage("Certain other options are available as, for instance, ");
  ph.add_usage("choosing to pipe the output to a file, setting the level of ");
  ph.add_usage("verbosity, and others; all these options will become");
  ph.add_usage("obvious from the following list of available flags, ");
  ph.add_usage("registered to the program.");

  // (Optional) We finally add some usage description, again line by line.
  ph.add_usage_description("This is the usage description text.");
  ph.add_usage_description("Please, feel free to run the program as often as");
  ph.add_usage_description("you like.");
  ph.add_usage_description("");
  ph.add_usage_description("Parameters: ");
  ph.add_usage_description(" + mode (required)");
  ph.add_usage_description("   Integer that can set some mode.");
  ph.add_usage_description(" + catch-juice (optional)");
  ph.add_usage_description("   Boolean, standard is false. ");
  ph.add_usage_description("   If true, juice is collected. ");
  ph.add_usage_description("");
  ph.add_usage_description("For instance, you could call: ");
  //*****>> Printing 4 lines >>
  std::string info_line(" > ");
  info_line.append(args[0]);
  info_line.append(" -h");
  ph.add_usage_description(info_line);
  info_line = std::string(" > ");
  info_line.append(args[0]);
  info_line.append("");
  ph.add_usage_description(info_line);
  info_line = std::string(" > ");
  info_line.append(args[0]);
  info_line.append(" -p mode=1 -e params.log");
  ph.add_usage_description(info_line);
  info_line = std::string(" > ");
  info_line.append(args[0]);
  info_line.append(" -f params.log");
  ph.add_usage_description(info_line);
  info_line = std::string(" > ");
  info_line.append(args[0]);
  info_line.append(" -p mode=2 -f params.log -p mode=3 foo -p some_thing=True");
  ph.add_usage_description(info_line);
  info_line = std::string(" > ");
  info_line.append(args[0]);
  info_line.append(" -f params.log CONTINUE -F apple 9 90 -F cherry one");
  ph.add_usage_description(info_line);
  //***** <<<<
  ph.add_usage_description("");
  ph.add_usage_description("Enjoy future use of the ParameterHandler.");
  std::string some_line("(C) Copyright 2021 Andreas Härtel");
  some_line.append(" (http://andreashaertel.anno1982.de/)");
  ph.add_usage_description(some_line);

  // Handle the flag "h","help":
  // If set, the function   void show_usage(void)   is called and the program
  // is stopped.
  // Note that ph.show_usage() can be called at any time to show all
  // information. Using show_usage(std::ostream& output) the output can also
  // be redirected in other streams that std::out.
  ph.process_flag_help();

  // Handle the flags "f","file" and "p","param":
  // All parameters are collected from an optional parameter file
  // (flag "f","file") and from the command line (flag "p","param").
  // This function can be called several times. For instance, after reading
  // a parameter like mode it could be decided to read another parameter as
  // required or as optional.
  // If this function is not called, no parameter is available by the
  // different get_xxx functions.
  ph.process_parameters();

  // Handle the flag "e","export"
  ph.export_parameters();


  // Get values of parameters.
  // Possible types are int,double,bool,char*,std::string.
  // Get the required parameter "mode".
  // Catch BadParamException, if "mode" is not defined.
  int mode = 0;
  try {
    mode = ph.get_int("mode");
  } catch (std::exception *e) {
    std::cout << " Exception message: " << e->what() << std::endl;
    // typically, you would stop the program here.
  }
  // Get the optional parameter "catch-juice" with standard value "false"
  bool catch_juice = ph.get_bool("catch-juice", false);
  // Inform user
  std::cout << " Mode set to: " << mode << std::endl;
  if (catch_juice) std::cout << " Juice will be catched. " << std::endl;
  else
    std::cout << " Juice will not be catched. " << std::endl;






  //#####################################################################
  //#####################################################################
  // Second part of the example
  //
  std::cout << std::endl << "To continue the programm, call it again and add"
            << " \"CONTINUE\" to the command line, i.e.: " << std::endl;
  std::string cmd_line(" > ");
  cmd_line.append(args[0]);
  cmd_line.append(" -p mode=1 CONTINUE");
  std::cout << cmd_line << std::endl;
  // Register "CONTINUE" as a flag with no additional option
  ph.register_flag("-C", "CONTINUE", 0);
  if (!ph.find_flag("-C")) return 0;
  //
  //#####################################################################
  // continue ...
  std::cout << std::endl
            << ">>> continue >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
            << std::endl << std::endl;





  // We still need to read in the fruits.
  // For this purpose, we register a new flag called "fruit".
  // The short flag name "f" is already registered, thus, we use "F".
  // Each fruit needs a name and a size; accordingly, we register 2 options.
  // Note that we add "-" and "--" here but, in general, we must not.
  //
  // void register_flag(const char *short_flag, const char *long_flag,
  //       int no_options);
  ph.register_flag("-F", "--fruit", 2);
  // We further add a description to the flag:
  // void add_flag_description(const char *short_flag, const char *description);
  ph.add_flag_description("-F", "Specify a fruit with <opt1> its name and");
  ph.add_flag_description("-F", "<opt2> its size in cm. ");

  // We can check, whether the flag has been registered previously:
  if (ph.is_flag("-F")) {
    std::cout << " The flag \"-F\" has been registered." << std::endl;
  }
  if (ph.is_flag("-fruit")) {
    std::cout << " The flag \"--fruit\" has been registered." << std::endl;
  }
  if (!ph.is_flag("F")) {
    std::cout << " The flag \"F\" has not been registered." << std::endl;
  }

  // If you call "--help", you will not see the new flag, because you processed
  // the flag "h" earlier than registering the new flag "F".
  // Thus, we should process "h" after registering all flags.
  // However, we can manually show the usage information now that would be
  // shown, if we would process "h" later:
  std::cout << std::endl << std::endl << "The help text, called manually: "
            << std::endl;
  ph.show_usage();
  std::cout << std::endl;


  // Finally, lets look for fruits:
  // First, we count them:
  int flag_pos = 0;
  int counter = 0;
  while (ph.find_flag("-F", &flag_pos)) counter++;
  std::cout << " We found " << counter << " fruits. " << std::endl;
  // Second, get their names and sizes:
  counter = 0;
  flag_pos = 0;
  while (ph.find_flag("-F", &flag_pos)) {
    counter++;
    std::string fruit_name = ph.get_option(flag_pos, 1);
    std::string fruit_size = ph.get_option(flag_pos, 2);
    std::cout << " + fruit " << counter << " : " << fruit_name
              << " (size " << fruit_size << " cm)" << std::endl;
  }

  // Remaining:
  std::cout << " Now, it is your turn to sort the fruits. Have fun. "
            << std::endl
            << " By the way: The remaining command line arguments that "
            << "have not been processed yet" << std::endl
            << " (this includes the name of the program) are: " << std::endl;
  // Get remaining, not processed, arguments:
  std::vector<std::string> remaining = ph.get_remaining_cmdline_arguments();
  // Using a for loop with iterator
  for (auto it = std::begin(remaining); it != std::end(remaining); ++it) {
    std::cout << " " << *it;
  }
  std::cout << std::endl;


  // Exit the program with return code 0
  std::cout << " The END." << std::endl;
  return 0;
}







