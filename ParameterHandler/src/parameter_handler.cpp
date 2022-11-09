// SPDX-FileCopyrightText: 2008-2021 Andreas Härtel <http://andreashaertel.anno1982.de/> // NOLINT
// SPDX-FileCopyrightText: 2020-2021 Moritz Bültmann <moritz.bueltmann@gmx.de> // NOLINT
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-PackageName: parameter_handler
// SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler
#include <regex>  // NOLINT
#include <iostream>
#include <fstream>
#include <ctime>
#include <exception>
#include <limits>
#include <utility>
#include "parameter_handler.hpp"
// _____________________________________________________________________________
ParameterHandler::ParameterHandler(int argc, char **args) {
  // Set number of command line arguments
  this->argc = argc;
  // Store command line arguments in a vector
  for (int i = 0; i < argc; ++i) {
    (this->args).push_back(args[i]);
  }
  // Register default flags
  register_flag("-h", "--help", 0);  // show usage information
  register_flag("-f", "--file", 1);  // specify parameter file
  register_flag("-p", "--param", 1);  // specify a parameter
  register_flag("-e", "--export", 1);  // specify file to export parameters
  // Add default description to the registered flags:
  add_flag_description("-h", "Show usage information.");
  add_flag_description("-f", "opt1 specifies the parameter file, from which.");
  add_flag_description("-f", "parameters are red. File format is: ");
  add_flag_description("-f", " <parameter name> <value>");
  add_flag_description("-f", "Repeating copies of a parameter are ignored.");
  add_flag_description("-f", "Text after a # is ignored up to the line end.");
  add_flag_description("-p", "opt1 specifies a parameter, using the format:");
  add_flag_description("-p", " opt1=<parameter name>=<value>");
  add_flag_description("-p", "Parameters specified already in the parameter ");
  add_flag_description("-p", "file are overwritten. ");
  add_flag_description("-p", "Repeating definitions of parameters overwrite ");
  add_flag_description("-p", "previous definitions (only the last ");
  add_flag_description("-p", "specification is used. ");
  add_flag_description("-e", "If specified, all used parameters are exported");
  add_flag_description("-e", "to the file specified by opt1. ");
  // Initialize usage information
  init_usage();
  return;
}
// _____________________________________________________________________________
ParameterHandler::~ParameterHandler(void) {
  // Remove elements from all vectors:
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    (it->description).clear();
  }
  flag_list.clear();  // clear the flag_list vector
  args.clear();  // clear the command line parameter vector
  usage_text.clear();
  parameters.clear();
  usage_descriptions.clear();
  return;
}
//##############################################################################
// Handle flags
//##############################################################################
// _____________________________________________________________________________
void ParameterHandler::register_flag(const char *short_flag,
        const char *long_flag, int no_options) {
  // Pass arguments to the register_flag(string,string,int) function
  std::string str1(short_flag);
  std::string str2(long_flag);
  register_flag(str1, str2, no_options);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::register_flag(std::string short_flag,
        std::string long_flag, int no_options) {
  flag new_flag;
  new_flag.short_name = short_flag;
  new_flag.long_name = long_flag;
  if (no_options < 0) {
    no_options = 0;
    std::cout << " WARNING:ParameterHandler::register_flag:: number of options"
              << " must be positiv or 0: reset to 0." << std::endl;
  }
  new_flag.no_options = no_options;
  flag_list.push_back(new_flag);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_flag_description(const char *short_flag,
        const char * description) {
  // Pass arguments to the add_flag_description(string,string) function
  std::string str1(short_flag);
  std::string str2(description);
  add_flag_description(str1, str2);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_flag_description(std::string short_flag,
        std::string description) {
  // Search flag_list for short_flag:
  auto it = flag_list.begin();
  while ((it != flag_list.end()) &&
         ((it->short_name).compare(short_flag) != 0)) {
    ++it;
  }
  // short_flag found:
  if (it != flag_list.end()) {
    (it->description).push_back(description);
    return;
  }
  // short flag not found:
  throw "ParameterHandler::add_flag_description:: flag not found";
  return;
}
// _____________________________________________________________________________
bool ParameterHandler::is_flag(const char *argument) {
  // Pass arguments to the is_flag(string) function
  std::string str(argument);
  return is_flag(str);
}
// _____________________________________________________________________________
bool ParameterHandler::is_flag(std::string argument) {
  // loop through the flag_list and search for argument
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    if (argument.compare(it->short_name) == 0) { return true; }
    if (argument.compare(it->long_name) == 0) { return true; }
  }
  return false;
}
// _____________________________________________________________________________
bool ParameterHandler::find_flag(int* flag_pos) {
  bool found_flag = false;
  auto it = args.begin();
  // Skip the first flag_pos+1 entries of args. If the end of args is reached,
  // it will immediately return that the flag could not be found.
  if (*flag_pos + 1 < static_cast<int>(args.size())) {
    it += *flag_pos + 1;
  } else {
    return found_flag;
  }
  // Find a flag
  while (it != args.end() && !found_flag) {
    // Test, whether args[it] is a flag in flag_list
    for (auto flag_it = flag_list.begin(); flag_it != flag_list.end();
         ++flag_it) {
      if ((it->compare(flag_it->short_name) == 0) ||
          (it->compare(flag_it->long_name) == 0)) {
        found_flag = true;
        // update flag_pos
        *flag_pos = it - args.begin();
        // test for requested number of options available (and not flags)
        auto it2 = it;
        for (int i = 0; (i < flag_it->no_options) && (it2 != args.end()); ++i) {
          ++it2;
          if ((it2 == args.end()) || is_flag(*it2)) {
            // requested option not available
            throw &bad_option;
          }
        }
      }
    }
    ++it;
  }
  return found_flag;
}
// _____________________________________________________________________________
bool ParameterHandler::find_flag(std::string flag_name, int* flag_pos) {
  // test flag_name specifying a flag, store both flags in strings:
  std::string flag_name_short(flag_name);
  std::string flag_name_long(flag_name);
  bool flag_found = false;
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    if (flag_name.compare(it->short_name) == 0) {
      flag_name_long = it->long_name;
      flag_found = true;
    }
    if (flag_name.compare(it->long_name) == 0) {
      flag_name_short = it->short_name;
      flag_found = true;
    }
  }
  if (!flag_found) {
    std::cout << " WARNING:ParameterHandler::find_flag: flag_name is not a flag"
              << std::endl;
    return false;
  }
  // search for any flag, then test for flag having flag_name
  int any_flag_pos = *flag_pos;
  while (find_flag(&any_flag_pos)) {
    // test for flag having flag_name
    if ( (flag_name_short.compare(args.at(any_flag_pos)) == 0) ||
         (flag_name_long.compare(args.at(any_flag_pos)) == 0) ) {
      // the flag has flag_name:
      *flag_pos = any_flag_pos;
      return true;
    }
  }
  // flag not found:
  return false;
}
// _____________________________________________________________________________
bool ParameterHandler::find_flag(std::string flag_name) {
  // Pass arguments to the find_flag(string, int*) function
  int tmp = 0;
  return find_flag(flag_name, &tmp);
}
// _____________________________________________________________________________
std::string ParameterHandler::find_option(const char *flag_name,
        int option_index) {
  int flag_pos = 0;
  std::string str1(flag_name);
  return find_option(str1, flag_pos, option_index);
}
// _____________________________________________________________________________
std::string ParameterHandler::find_option(std::string flag_name,
        int option_index) {
  int flag_pos = 0;
  return find_option(flag_name, flag_pos, option_index);
}
// _____________________________________________________________________________
std::string ParameterHandler::find_option(const char *flag_name,
        int flag_pos, int option_index) {
  std::string str1(flag_name);
  return find_option(str1, flag_pos, option_index);
}
// _____________________________________________________________________________
std::string ParameterHandler::find_option(std::string flag_name,
        int flag_pos, int option_index) {
  // validity check
  std::string flag_name_short(flag_name);
  std::string flag_name_long(flag_name);
  bool flag_found = false;
  int number_of_options = 0;
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    if (flag_name.compare(it->short_name) == 0) {
      flag_name_long = it->long_name;
      number_of_options = it->no_options;
      flag_found = true;
    }
    if (flag_name.compare(it->long_name) == 0) {
      flag_name_short = it->short_name;
      number_of_options = it->no_options;
      flag_found = true;
    }
  }
  // Check, whether flag_name is a registered flag:
  if (!flag_found) {
    std::cout << " WARNING:ParameterHandler::find_option: flag_name is"
              << " not a flag" << std::endl;
    std::string tmp("");
    return tmp;
  }
  // check, whether option_index is a valid number:
  if ((option_index < 1) || (option_index > number_of_options)) {
    std::cout << " WARNING:ParameterHandler::find_option: option not available"
              << std::endl;
    std::string tmp("");
    return tmp;
  }
  // find flag:
  if (!find_flag(flag_name, &flag_pos)) {
    throw &bad_param;
  }
  // return option:
  return args.at(flag_pos+option_index);
}
// _____________________________________________________________________________
std::string ParameterHandler::get_option(int flag_pos, int option_index) {
  // check validity of flag_pos:
  if ((flag_pos < 0) || (flag_pos >= unsigned_to_signed_int(args.size()))) {
    std::cout << " WARNING:ParameterHandler::get_option: invalid flag_pos"
              << std::endl;
    std::string tmp("");
    return tmp;
  }
  // set flag_name
  std::string flag_name(args.at(flag_pos));
  // check, if this is a flag:
  if (!is_flag(flag_name)) {
    std::cout << " WARNING:ParameterHandler::get_option: invalid flag_pos"
              << std::endl;
    std::string tmp("");
    return tmp;
  }
  // validity check for option_index:
  std::string flag_name_short(flag_name);
  std::string flag_name_long(flag_name);
  bool flag_found = false;
  int number_of_options = 0;
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    if (flag_name.compare(it->short_name) == 0) {
      flag_name_long = it->long_name;
      number_of_options = it->no_options;
      flag_found = true;
    }
    if (flag_name.compare(it->long_name) == 0) {
      flag_name_short = it->short_name;
      number_of_options = it->no_options;
      flag_found = true;
    }
  }
  if (!flag_found) { std::cout << "STRANGE ERROR!" << std::endl; exit(0); }
  // check, whether option_index is a valid number:
  if ((option_index < 1) || (option_index > number_of_options)) {
    std::cout << " WARNING:ParameterHandler::find_option: option not available"
              << std::endl;
    std::string tmp("");
    return tmp;
  }
  // search for flag with correct flag_pos:
  int search_pos = 0;
  while (find_flag(flag_name, &search_pos)) {
    if (search_pos == flag_pos) {
      // flag found: get option:
      return args.at(flag_pos+option_index);
    }
  }
  // option not found:
  std::cout << " WARNING:ParameterHandler::find_option: option not available"
            << std::endl;
  std::string tmp("");
  return tmp;
}
// _____________________________________________________________________________
std::vector<std::string> ParameterHandler::get_remaining_cmdline_arguments() {
  // new vector for remaining arguments
  std::vector<std::string> rem_args;
  int any_flag_pos = 0;
  int last_no_flag_pos = 0;
  // search next flag
  while (find_flag(&any_flag_pos)) {
    // flag found
    // store remaining arguments in front of new flag
    for (; last_no_flag_pos < any_flag_pos; last_no_flag_pos++) {
      rem_args.push_back(args.at(last_no_flag_pos));
    }
    // skip the found flag
    last_no_flag_pos++;
    // skip options belonging to the found flag
    // get flag information (first: search the corresponding flag)
    int number_flag_options = 0;
    for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
      if (((it->short_name).compare(args.at(any_flag_pos)) == 0) ||
          ((it->short_name).compare(args.at(any_flag_pos)) == 0)) {
        number_flag_options = it->no_options;
      }
    }
    last_no_flag_pos = last_no_flag_pos + number_flag_options;
  }
  // store remaining arguments at the end (after the last flag)
  for (; last_no_flag_pos < unsigned_to_signed_int(args.size());
       last_no_flag_pos++) {
    rem_args.push_back(args.at(last_no_flag_pos));
  }
  // return the list
  return rem_args;
}
//##############################################################################
// Handle usage information
//##############################################################################
// _____________________________________________________________________________
void ParameterHandler::init_usage(void) {
  std::string str("");
  str += " Usage: " + args.at(0) + " [<flag> <opt1 ...> <flag> <opt1 ...>]";
  usage_text.push_back(str);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_usage(const char *line) {
  // Pass arguments to the add_usage(string) function
  std::string str(line);
  add_usage(str);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_usage(std::string line) {
  usage_text.push_back(line);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_usage_description(const char *line) {
  // Pass arguments to the add_usage_description(string) function
  std::string str(line);
  add_usage_description(str);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_usage_description(std::string line) {
  usage_descriptions.push_back(line);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::show_usage(void) {
  show_usage(std::cout);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::show_usage(std::ostream& output) {
  // Show usage text
  for (auto it = usage_text.begin(); it != usage_text.end(); ++it) {
    output << " " << *it << std::endl;
  }
  // Show flag information
  output << "" << std::endl;
  output << " Flags and their options: " << std::endl;
  // search for maximal flag length:
  int max_len = 0;
  int tmp_len;
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    tmp_len = (it->short_name).length() + (it->long_name).length();
    if (tmp_len > max_len) { max_len = tmp_len; }
  }
  // loop over flags:
  for (auto it = flag_list.begin(); it != flag_list.end(); ++it) {
    tmp_len = (it->short_name).length() + (it->long_name).length();
    output << "  " << it->short_name << " " << it->long_name << " ";
    // fill line up to maximal flag length:
    for (; tmp_len < max_len; tmp_len++) { output << " "; }
    // write first description line by hand:
    auto desc_it = (it->description).begin();
    // write first entry, if exist
    if (desc_it != (it->description).end()) {
      output << *desc_it << std::endl;
      ++desc_it;
    } else {
      // no entry available, write just endl
      output << std::endl;
    }
    // loop over remaining description vector
    for (; desc_it != (it->description).end(); ++desc_it) {
      // start a line with spaces:
      for (int i = 0; i < max_len+4; i++) { output << " "; }
      // write a description line:
      output << *desc_it << std::endl;
    }
  }
  output << "" << std::endl;
  // Show description text
  for (auto it = usage_descriptions.begin(); it != usage_descriptions.end();
            ++it) {
    output << " " << *it << std::endl;
  }
  return;
}
//##############################################################################
// Process arguments (and add parameters)
//##############################################################################
// _____________________________________________________________________________
void ParameterHandler::process_flag_help(void) {
  if (find_flag("-h")) {
    show_usage();
    exit(0);
  }
  return;
}
// _____________________________________________________________________________
void ParameterHandler::process_parameters(void) {
  // remove all previously stored parameters
  while (parameters.begin() != parameters.end()) {
    parameters.pop_back();
  }
  // Add parameters from file, if specified
  try {
    add_parameters_from_file();
  }
  catch (BadOptionException *e) {  }
  catch (std::exception *e) {
    std::cout << " ERROR::ParameterHandler::add_file: " << e->what()
              << std::endl;
  }
  // Add parameters specified in args via the command line
  add_parameters_from_cmdline();
  // Export parameters to file if the flag is present
  try {
    export_parameters();
  }
  catch (BadOptionException *e) {  }
  catch (std::exception *e) {
    std::cout << " ERROR::ParameterHandler::export_parameters: ";
    std::cout << e->what() << std::endl;
  }
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_parameters_from_file(void) {
  // Check for a specified parameter file using the flags "-f" or "--file"
  int flag_pos = 0;
  if (find_flag("-f", &flag_pos)) {
    // find flag has checked already for the existence of a corresponding option
    // If the flag is set without an option specified, a bad_option exception
    // is thrown
    // Set file name and load parameters from file
    std::string inFileName = args.at(flag_pos+1);
    load_parameter_file(inFileName);
  }
  return;
}
// _____________________________________________________________________________
void ParameterHandler::load_parameter_file(std::string filename) {
  // Dummy string
  std::string line;
  // Regular expression to filter comments beforehand
  std::regex comment_regex("^([^#]*)");
  // Regular expression to get name-value pairs
  std::regex param_regex("^\\s*(\\w+)\\s+([^\\s]+)\\s*");
  // Make smatch object containing the filtered words
  std::smatch param_match;
  std::smatch comment_match;
  // Try to open the parameter file
  try {
    std::fstream parameterFile(filename, std::ios::in);
    if (parameterFile.is_open()) {
      while (getline(parameterFile, line)) {
        // read file line by line
        // filter out comments:
        std::regex_search(line, comment_match, comment_regex);
        line = comment_match.str(1);
        // filter for a parameter
        if (std::regex_search(line, param_match, param_regex)) {
          // Add the name-value pair to the other pairs (if not yet defined)
          int duplicate_pos = find_parameter_position(param_match.str(1));
          if (duplicate_pos < 0) {
            // parameter is new: add new entry to parameters:
            parameters.push_back(make_pair(param_match.str(1),
                                           param_match.str(2)));
          }
        }
      }
    } else {
      // file cannot be opened: throw error
      throw &bad_file;
    }
  } catch (std::exception *e) {
    std::cout << " ERROR:ParameterHandler: " << e->what() << std::endl;
  }
  return;
}
// _____________________________________________________________________________
void ParameterHandler::add_parameters_from_cmdline(void) {
  // Regular expression to get name-value pairs from the command line arguments
  std::regex param_regex("^(\\w+)=([^\\s]+)$");
  std::smatch param_match;
  // Find all parameters specified by "-p" or "--params" flag
  int flag_pos = 0;
  bool flag_found = false;
  do {
    // find new parameter flag (and check for option available)
    try {
      flag_found = find_flag("-p", &flag_pos);
      if (!flag_found) { continue; }
      // flag found and
      // option is available (otherwise an exception has been thrown)
      // +++++++++++++++++++++++++++++++++++++++++++++++++++
      // Match the found parameter-value pair:
      if (std::regex_search(args.at(flag_pos+1), param_match, param_regex)) {
        // parameter-value pair found:
        // check for occurence of parameter in parameters vector:
        int duplicate_pos = find_parameter_position(param_match.str(1));
        if (duplicate_pos < 0) {
          // add new entry to parameters:
          parameters.push_back(make_pair(param_match.str(1),
                                         param_match.str(2)));
        } else {
          // replace existing occurence:
          (parameters.at(duplicate_pos)).second = param_match.str(2);
        }
      } else {
        // parameter flag option does not match the expected format
        std::cout << " WARNING:ParameterHandler: option of flag -p could not";
        std::cout << " be matched to name=value. " << std::endl;
      }
      // ---------------------------------------------------
    } catch (BadOptionException *e) {
      // No option (parameter-value pair) specified for the flag
      std::cout << " WARNING:ParameterHandler: flag -p set without parameter.";
      std::cout << std::endl;
    }
  } while (flag_found);
  return;
}
// _____________________________________________________________________________
void ParameterHandler::export_parameters(void) {
  // Search for an "-e" or "--export" flag:
  int flag_pos = 0;
  if (find_flag("-e", &flag_pos)) {
    // flag found (and option does exist, otherwise an exception is thrown):
    std::string outFileName = args.at(flag_pos+1);  // set file name
    std::fstream parameterFile;  // define file stream object
    // Open file stream
    try {
      parameterFile.open(outFileName, std::ios::out);
      // Print command, that the file was created with.
      time_t now = time(0);
      parameterFile << "# File generated on UTC " << asctime(gmtime(&now));  // NOLINT
      parameterFile << "# by command: " << std::endl;
      parameterFile << "# ";
      for (auto it = args.begin(); it != args.end(); ++it) {
        parameterFile << " " << *it;
      }
      parameterFile << std::endl;
      parameterFile << std::endl;
      // Write parameters:
      for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        parameterFile << it->first << " " << it->second << std::endl;
      }
      parameterFile << std::endl;
      parameterFile.close();
    } catch (std::exception *e) {
      std::cout << " ERROR:ParameterHandler::export_parameters: " << e->what();
      std::cout << std::endl;
    }
  }
  return;
}
//##############################################################################
// Handle parameters (GET PARAMETER FUNCTIONS)
//##############################################################################
// _____________________________________________________________________________
int ParameterHandler::find_parameter_position(const std::string parameter) {
  // Search for parameter in vector parameters:
  for (auto it = parameters.begin(); it != parameters.end(); ++it) {
    if ((it->first).compare(parameter) == 0) {
      // parameter found
      return (it - parameters.begin());
    }
  }
  return -1;
}
// _____________________________________________________________________________
bool ParameterHandler::isset_parameter(std::string parameter) {
  int pos = find_parameter_position(parameter);
  return (pos >= 0);
}
// _____________________________________________________________________________
std::string ParameterHandler::get_value(std::string parameter) {
  int pos = find_parameter_position(parameter);
  if ((pos < 0) || (pos >= unsigned_to_signed_int(parameters.size()))) {
    // position does not exist!
    throw &bad_param;
  }
  return (parameters.at(pos)).second;
}
// _____________________________________________________________________________
double ParameterHandler::get_double(const char *parameter) {
  std::string str(parameter);
  return get_double(str);
}
// _____________________________________________________________________________
double ParameterHandler::get_double(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return stod(get_value(parameter));
  }
  // throw exception
  std::cout << " Parameter " << parameter << " not defined. " << std::endl;
  throw &bad_param;
  return 0.0;
}
// _____________________________________________________________________________
double ParameterHandler::get_double(const char *parameter, double standard) {
  std::string str(parameter);
  return get_double(str, standard);
}
// _____________________________________________________________________________
double ParameterHandler::get_double(std::string parameter, double standard) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_double(parameter);
  }
  return standard;
}
// _____________________________________________________________________________
int ParameterHandler::get_int(const char *parameter) {
  std::string str(parameter);
  return get_int(str);
}
// _____________________________________________________________________________
int ParameterHandler::get_int(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return stoi(get_value(parameter));
  }
  // throw exception
  std::cout << " Parameter " << parameter << " not defined. " << std::endl;
  throw &bad_param;
  return 0;
}
// _____________________________________________________________________________
int ParameterHandler::get_int(const char *parameter, int standard) {
  std::string str(parameter);
  return get_int(str, standard);
}
// _____________________________________________________________________________
int ParameterHandler::get_int(std::string parameter, int standard) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_int(parameter);
  }
  return standard;
}
// _____________________________________________________________________________
bool ParameterHandler::get_bool(const char *parameter) {
  std::string str(parameter);
  return get_bool(str);
}
// _____________________________________________________________________________
bool ParameterHandler::get_bool(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    std::string str(get_value(parameter));
    if (str.compare("false") == 0) { return false; }
    if (str.compare("true") == 0) { return true; }
    if (str.compare("False") == 0) { return false; }
    if (str.compare("True") == 0) { return true; }
    if (str.compare("FALSE") == 0) { return false; }
    if (str.compare("TRUE") == 0) { return true; }
    if (str.compare("0") == 0) { return false; }
    if (str.compare("1") == 0) { return true; }
    // Could not detect a bool:
    // throw exception
    std::cout << " Parameter " << parameter << " has wrong type. " << std::endl;
    throw &bad_param;
    return false;
  }
  // throw exception
  std::cout << " Parameter " << parameter << " not defined. " << std::endl;
  throw &bad_param;
  return false;
}
// _____________________________________________________________________________
bool ParameterHandler::get_bool(const char *parameter, bool standard) {
  std::string str(parameter);
  return get_bool(str, standard);
}
// _____________________________________________________________________________
bool ParameterHandler::get_bool(std::string parameter, bool standard) {
  if (isset_parameter(parameter)) {
    return get_bool(parameter);
  }
  return standard;
}
// _____________________________________________________________________________
char* ParameterHandler::get_char(const char *parameter) {
  std::string str(parameter);
  return get_char(str);
}
// _____________________________________________________________________________
char* ParameterHandler::get_char(std::string parameter) {
  if (isset_parameter(parameter)) {
    std::string str(get_value(parameter));
    // Allocate memory to make the char* last
    try {
      char *value;
      value = new char[str.size()];
      // Copy value into char*
      strcpy(value, str.c_str());  // NOLINT
      return value;
    } catch (std::exception *e) {
      std::cout << " ERROR:ParameterHandler::get_char: " << e->what();
      std::cout << std::endl;
    }
  }
  // throw exception
  std::cout << " Parameter " << parameter << " not defined. " << std::endl;
  throw &bad_param;
  return NULL;
}
// _____________________________________________________________________________
char* ParameterHandler::get_char(const char *parameter, char *standard) {
  std::string str(parameter);
  return get_char(str, standard);
}
// _____________________________________________________________________________
char* ParameterHandler::get_char(std::string parameter, char *standard) {
  if (isset_parameter(parameter)) {
    return get_char(parameter);
  }
  return standard;
}
// _____________________________________________________________________________
std::string ParameterHandler::get_string(const char *parameter) {
  std::string str(parameter);
  return get_string(str);
}
// _____________________________________________________________________________
std::string ParameterHandler::get_string(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_value(parameter);
  }
  // throw exception
  std::cout << " Parameter " << parameter << " not defined. " << std::endl;
  throw &bad_param;
  std::string tmp;
  return tmp;
}
// _____________________________________________________________________________
std::string ParameterHandler::get_string(const char *parameter, std::string
              standard) {
  std::string str(parameter);
  return get_string(str, standard);
}
// _____________________________________________________________________________
std::string ParameterHandler::get_string(std::string parameter, std::string
              standard) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_string(parameter);
  }
  return standard;
}
// _____________________________________________________________________________
std::vector<int> ParameterHandler::get_vector_int(const char* parameter) {
  std::string str(parameter);
  return get_vector_int(str);
}
// _____________________________________________________________________________
std::vector<int> ParameterHandler::get_vector_int(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available: 
    return string_to_vector_int(get_value(parameter));
  } else {
    // throw exception
    std::cout << " Parameter " << parameter << " not defined. " << std::endl;
    throw &bad_param;
    return std::vector<int>();
  }
}
// _____________________________________________________________________________
std::vector<int> ParameterHandler::get_vector_int(
    const char* parameter, std::vector<int> standard
) {
  std::string str(parameter);
  return get_vector_int(str, standard);
}
// _____________________________________________________________________________
std::vector<int> ParameterHandler::get_vector_int(
    std::string parameter, std::vector<int> standard
) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_vector_int(parameter);
  } else {
    return standard;
  }
}
// _____________________________________________________________________________
std::vector<double> ParameterHandler::get_vector_double(const char* parameter) {
  std::string str(parameter);
  return get_vector_double(str);
}
// _____________________________________________________________________________
std::vector<double> ParameterHandler::get_vector_double(std::string parameter) {
  if (isset_parameter(parameter)) {
    // Parameter available: 
    return string_to_vector_double(get_value(parameter));
  } else {
    // throw exception
    std::cout << " Parameter " << parameter << " not defined. " << std::endl;
    throw &bad_param;
    return std::vector<double>();
  }
}
// _____________________________________________________________________________
std::vector<double> ParameterHandler::get_vector_double(
    const char* parameter, std::vector<double> standard
) {
  std::string str(parameter);
  return get_vector_double(str, standard);
}
// _____________________________________________________________________________
std::vector<double> ParameterHandler::get_vector_double(
    std::string parameter, std::vector<double> standard
) {
  if (isset_parameter(parameter)) {
    // Parameter available:
    return get_vector_double(parameter);
  } else {
    return standard;
  }
}
// _____________________________________________________________________________
std::vector<int> ParameterHandler::string_to_vector_int(std::string str) {
  // Dummy for return
  std::vector<int> converted;
  // Dummy string
  std::string regex_target(str);
  // Regular expression to get the elements of the parameter vector
  std::regex param_regex("([^\\s^,]+)");
  std::smatch param_match;
  // Match vector elements until all of them are captured.
  while(std::regex_search(regex_target, param_match, param_regex)) {
    // Add the match to the return vector.
    converted.push_back(stoi(param_match.str(1)));
    // Remove the match from the target string.
    regex_target = param_match.suffix();
  }
  if (converted.size() == 0){
    throw "ParameterHandler::string_to_vector_int: could not find any";
    throw " elements in given string.";
  }
  return converted;
}
// _____________________________________________________________________________
std::vector<double> ParameterHandler::string_to_vector_double(std::string str) {
  // Dummy for return
  std::vector<double> converted;
  // Dummy string
  std::string regex_target(str);
  // Regular expression to get the elements of the parameter vector
  std::regex param_regex("([^\\s^,]+)");
  std::smatch param_match;
  // Match vector elements until all of them are captured.
  while(std::regex_search(regex_target, param_match, param_regex)) {
    // Add the match to the return vector.
    converted.push_back(stod(param_match.str(1)));
    // Remove the match from the target string.
    regex_target = param_match.suffix();
  }
  if (converted.size() == 0){
    throw "ParameterHandler::string_to_vector_double: could not find any";
    throw " elements in given string.";
  }
  return converted;
}
// _____________________________________________________________________________
int ParameterHandler::unsigned_to_signed_int(unsigned int input) {
  if (input < std::numeric_limits<int>::max()) {
    return static_cast<int>(input);
  } else {
    throw "ParameterHandler::unsigned_to_signed_int:: unsigned int too large.";
    return -1;
  }
}
// _____________________________________________________________________________
