// SPDX-FileCopyrightText: 2008-2021 Andreas Härtel <http://andreashaertel.anno1982.de/> // NOLINT
// SPDX-FileCopyrightText: 2020-2021 Moritz Bültmann <moritz.bueltmann@gmx.de> // NOLINT
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-PackageName: parameter_handler
// SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler
/*******************************************************************************
* This is the improved version of Andreas' command line tool. Instead of the
* C-madness, I used some clean C++ classes and regular expressions. It should be
* completely compatible to the rest of Andreas' code.
* The version of Moritz has been improved further.
*******************************************************************************/
#ifndef SRC_PARAMETER_HANDLER_HPP_
#define SRC_PARAMETER_HANDLER_HPP_
/** \file parameter_handler.hpp
 *  \brief Header file for the ParameterHandler class.
 *
 *  The file contains the class definition of the ParameterHandler class.
 *  This contains the definition of class specific exceptions and the
 *  definition of a struct flag.
 */
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
// _____________________________________________________________________________
/** \brief ParameterHandler to manage flag and parameter handling.
 *
 *  The class works on the command line arguments that a C/C++ program receives
 *  in its main(int argc, char **args) function.
 *  It provides the functionality to register flags, like "-h" with a certain
 *  number of options. For instance, the flag "-f" is defined by default to
 *  provide a parameter file: the file name is the one option. The program call
 *  looks like [program] -f [file name].
 *  Each flag has a short and a long version (e.g. "-f" and "--file").
 *  User defined flags can be registered, too.
 *  Furthermore, the class allows to add usage information and descriptions,
 *  which are piped into an output stream (standard is the standard out stream)
 *  if the "-h" flag is set.
 *  Parameters can be defined not only in the parameter file, but also via the
 *  command line flag "-p"; parameters defined via the command line overwrite
 *  parameters from the parameter file.
 *  Finally, the standard flag "-e" allows to export all available parameters
 *  into a file.
 */
class ParameterHandler {
 public:
// _____________________________________________________________________________
  /** \brief Constructor
   *
   *  \param argc Number of elements in args, as passed to the main function.
   *  \param args Command line arguments, as passed to the main function.
   *
   *  Stores the arguments from args in the args vector and registers the
   *  default flags
   *   - register_flag("-h", "--help", 0), to show user information,
   *   - register_flag("-f", "--file", 1), to specify a parameter file,
   *   - register_flag("-p", "--param", 1), to specify parameter via the command
   *  line, and
   *   - register_flag("-e", "--export", 1), to specify a file to export all
   *  available parameters to.
   *  To handle the standard flags, the functions 
   *   - void process_flag_help(void);
   *   - void process_parameters(void);
   *   - void export_parameters(void);
   *  can be used; the flags are not handled automatically. 
   */
  ParameterHandler(int argc, char **args);
// _____________________________________________________________________________
  /** \brief Destructor
   *
   *  Clears all vectors. */
  ~ParameterHandler(void);
//##############################################################################
// Handle flags
//##############################################################################
// _____________________________________________________________________________
  /** \brief Registers a flag.
   *
   *  Passes the arguments forward to the
   *  register_flag(std::string, std::string, int) function using std::string
   *  instead of const char *. */
  void register_flag(const char *short_flag, const char *long_flag,
         int no_options);
// _____________________________________________________________________________
  /** \brief Registers a flag.
   *
   *  \param short_flag The short form of the flag (e.g. "-h").
   *  \param long_flag The long form of the flag (e.g. "--help").
   *  \param no_options The number of options (command line arguments) that
   *         are expected to follow the flag: 0,1,2,... .
   *
   *  Sets up a new flag and adds it to the flag_list.
   */
  void register_flag(std::string short_flag, std::string long_flag,
         int no_options);
// _____________________________________________________________________________
  /** \brief Adds a description to a registered flag.
   *
   *  Passes the arguments forward to the
   *  add_flag_description(std::string, std::string) function using std::string
   *  instead of const char *. */
  void add_flag_description(const char *short_flag, const char * description);
  /** \brief Adds a description to a registered flag.
   *
   *  \param short_flag The short form of the flag.
   *  \param description The text line that is add to the description of the
   *         flag.
   *
   *  The flag must be registered before.
   *  In the show_usage function, the description is printed line by line, as
   *  added to the flag.
   *  We recommend to refer to options using opt<number> (e.g. opt1, opt2). */
  void add_flag_description(std::string short_flag, std::string description);
// _____________________________________________________________________________
  /** \brief Test whether argument is registered as a flag.
   *
   *  Passes the arguments forward to the
   *  is_flag(std::string) function using std::string instead of const char *.*/
  bool is_flag(const char *argument);
// _____________________________________________________________________________
  /** \brief Test whether argument is registered as a flag.
   *
   *  \param argument Argument that is checked to be a registered flag.
   *  \return true if argument is registered as a flag, false otherwise.
   *
   *  Test, whether the std::string argument is registered as a short or long
   *  version of a flag, i.e. whether it is stored in the flag_list vector. */
  bool is_flag(std::string argument);
// _____________________________________________________________________________
  /** \brief Search a registered flag in the arguments.
   *
   *  \param flag_pos Position in the args vector, after which the search
   *         starts. If a flag is found, the new position is stored in flag_pos.
   *  \return true if a flag is found after flag_pos, false otherwise.
   *
   *  The search starts after the given flag position in the args vector.
   *  If a flag is found, the flag position is updated.
   *  If the number of options registered for the found flag is not available,
   *  a bad_option exception is thrown.
   */
  bool find_flag(int* flag_pos);
// _____________________________________________________________________________
  /** \brief Search the specified registered flag in the arguments.
   *
   *  \param flag_name Short or long form of the flag that is searched for.
   *  \param flag_pos Position in the args vector, after which the search
   *         starts. If a flag is found, the new position is stored in flag_pos.
   *  \return true if a flag is found after flag_pos, false otherwise.
   *
   *  The search starts after the given flag position in the args vector.
   *  If a respective flag is found, the flag position is updated.
   *  If the number of options registered for the found flag is not available,
   *  a bad_option exception is thrown.
   */
  bool find_flag(std::string flag_name, int* flag_pos);
// _____________________________________________________________________________
  /** \brief Search the specified registered flag in the arguments.
   *
   *  \param flag_name Short or long form of the flag that is searched for.
   *  \return true if a flag is found, false otherwise.
   *
   *  The whole args vector is searched for the flag, starting at the beginning.
   *  The function performs like find_flag(flag_name, 0).
   *  If the number of options registered for the found flag is not available,
   *  a bad_option exception is thrown.
   */
  bool find_flag(std::string flag_name);
// _____________________________________________________________________________
  /** \brief Search a flag and return a specified option.
   *
   *  Passes the arguments forward to the
   *  find_option(std::string,int) function using std::string
   *  instead of const char *.
   */
  std::string find_option(const char *flag_name, int option_index);
// _____________________________________________________________________________
  /** \brief Search a flag and return a specified option.
   *
   *  \param flag_name Short or long form of the flag that is searched for.
   *  \param option_index The index of the option that is returned.
   *  \return the option of the found flag with index option_index. If flag_name
   *          is not a flag or option_index is not valid, the empty
   *          std::string "" is returned.
   *
   *  Searches for the flag flag_name in the args vector, starting from the
   *  begin. If flag_name is not a flag or option_index is not a valid index
   *  (<1 or larger than total number of options for the flag), the empty
   *  std::string "" is returned. If the flag is not found, a bad_param
   *  exception is thrown.
   */
  std::string find_option(std::string flag_name, int option_index);
// _____________________________________________________________________________
  /** \brief Search a flag and return a specified option.
   *
   *  Passes the arguments forward to the
   *  find_option(std::string,int,int) function using std::string
   *  instead of const char *.
   */
  std::string find_option(const char *flag_name, int flag_pos,
                         int option_index);
// _____________________________________________________________________________
  /** \brief Search a flag and return a specified option.
   *
   *  \param flag_name Short or long form of the flag that is searched for.
   *  \param flag_pos Position in the args vector, after which the search
   *         starts. If a flag is found, the new position is stored in flag_pos.
   *  \param option_index The index of the option that is returned.
   *  \return the option of the found flag with index option_index. If flag_name
   *          is not a flag or option_index is not valid, the empty
   *          std::string "" is returned.
   *
   *  Searches for the flag flag_name in the args vector, starting after the
   *  flag_pos position in the args vector. If flag_name is not a flag or
   *  option_index is not a valid index (<1 or larger than total number of
   *  options for the flag), the empty std::string "" is returned. If the
   *  flag is not found, a bad_param exception is thrown.
   */
  std::string find_option(std::string flag_name, int flag_pos,
                         int option_index);
// _____________________________________________________________________________
  /** \brief Returns the specified option to the flag at position flag_pos.
   *
   *  \param flag_pos The position in the args vector, at which the flag is
   *         assumed, for which the option will be returned.
   *  \param option_index The index of the option that is returned.
   *  \return the option of the flag at position flag_pos with option index
   *          option_index. If flag_pos does not specify a flag or option_index
   *          is not valid, the empty std::string "" is returned.
   *
   *  The function is perfect for usage together with the
   *  find_flag(std::string,int) function that sets the current position of the
   *  found flag:
   *
   *  ParameterHandler cmdtool(argc, args);
   *
   *  int flag_pos = 0;
   *
   *  while (cmdtool.find_flag("-f", flag_pos)) {
   *
   *    std::cout << cmdtool.get_option(flag_pos, 1) << std::endl;
   *
   *  }
   */
  std::string get_option(int flag_pos, int option_index);
// _____________________________________________________________________________
  /** \brief Returns a vector of the remaining command line arguments.
   *
   *  \return a vector of std::string of all command line arguments in the
   *          args vector that are not used for registered flags and their
   *          options.
   *
   *  All registered flags and their options are removed from a copy of the
   *  args vector. The resulting vector is returned.
   */
  std::vector<std::string> get_remaining_cmdline_arguments(void);
//##############################################################################
// Handle usage information
//##############################################################################
// _____________________________________________________________________________
  /** \brief Adds a text line to the usage text.
   *
   *  Passes the argument forward to the
   *  add_usage(std::string) function using std::string
   *  instead of const char *.
   */
  void add_usage(const char *line);
// _____________________________________________________________________________
  /** \brief Adds a text line to the usage text.
   *
   *  \param line Text line that is add to the usage text.
   *
   *  The line is add to the vector usage_text.
   *  The usage text is shown, when a show_usage function is called.
   *
   *  \note Note the difference between usage text and usage description text
   *  (compare with add_description(std::string) and
   *  see show_usage(std::ostream& output) for more details).
   */
  void add_usage(std::string line);
// _____________________________________________________________________________
  /** \brief Adds a text line to the usage description text.
   *
   *  Passes the argument forward to the
   *  add_usage_description(std::string) function using std::string
   *  instead of const char *.
  */
  void add_usage_description(const char *line);
// _____________________________________________________________________________
  /** \brief Adds a text line to the usage description text.
   *
   *  \param line Text line that is add to the usage description text.
   *
   *  The line is add to the vector usage_descriptions.
   *  The usage description text is shown, when a show_usage function is called.
   *
   *  \note Note the difference between usage text and usage description text
   *  (compare with add_usage(std::string) and
   *  see show_usage(std::ostream&) for more details).
   */
  void add_usage_description(std::string line);
// _____________________________________________________________________________
  /** \brief Pipe all usage information to the standard output.
   *
   *  Calls show_usage(std::ostream&) with std::cout as argument.
   */
  void show_usage(void);
// _____________________________________________________________________________
  /** \brief Pipe all usage information to output.
   *
   *  \param output The std::ostream, all available usage information is piped
   *                to.
   *
   *  The available usage information is given by:
   *  - the name of the program as it was called and the string
   *    " [<flag> <opt1 ...> <flag> <opt1 ...>]",
   *  - the usage text, as stored in the vector usage_text
   *    (see add_usage(std::string) function),
   *  - all registered flag names and their respective description in a table,
   *    i.e. short_name, long_name, description
   *    (see register_flag(std::string,std::string,int) and
   *    add_flag_description(std::string,std::string) functions),
   *  - the usage description text, as stored in the vector usage_descriptions
   *    (see add_description(std::string) function).
   *
   *  The usage information is piped in this order to the std::ostream output
   *  and is formated like a man page description.
   */
  void show_usage(std::ostream& output);
//##############################################################################
// Process arguments (and add parameters)
//##############################################################################
// _____________________________________________________________________________
  /** \brief Process the help flag.
   *
   *  If the standard flag (short_name "-h", long_name "--help") is set,
   *  the show_usage(void) function is called and the program is ended using
   *  exit(0).
   *
   *  Calling this function corresponds to the code:
   *
   *  if (find_flag("-h")) { show_usage(std::cout); return 0; }
   */
  void process_flag_help(void);
// _____________________________________________________________________________
  /** \brief Process the flags -f, -e, -p (handle parameters).
   *
   *  Handles parameters provided to the program via a parameter file
   *  (using the standard flag (short_name "-f", long_name "--file") and
   *  via parameter flags (short_name "-p", long_name "--param"); if the
   *  standard flag (short_name "-e", long_name "--export") is set, all
   *  parameters are additionally written to the specified export file.
   *
   *  First, the parameters vector is emptied (previous calls of this function
   *  are ignored: all parameters are re-procesed).
   *
   *  Second, the add_parameters_from_file(void) function is called that will
   *  import all parameters from a specified parameter file, if the standard
   *  flag (short_name "-f", long_name "--file") is set.
   *  Exceptions BadOptionException and std::exception thrown by
   *  add_parameters_from_file(void) are catched.
   *
   *  Third, the add_parameters_from_cmdline(void) function is called to add
   *  all parameters specified via the parameter flag in the command line.
   *
   *  Finally, the export_parameters(void) function is called that will
   *  export all imported parameters to a specified parameter file, if the
   *  standard flag (short_name "-e", long_name "--export") is set.
   *  Exceptions BadOptionException and std::exception thrown by
   *  export_parameters(void) are catched.
   */
  void process_parameters(void);
// _____________________________________________________________________________
  /** \brief Exports all imported parameters to a file, if flag -e is set.
   *
   *  If the standard flag (short_name "-e", long_name "--export") is set,
   *  all imported parameters that are stored in the vector parameters are
   *  exported to the specified file (specified via the flag option).
   *
   *  The function does not catch exception thrown by the
   *  find_flag(std::string,int) function that is used for getting the export
   *  file name.
   */
  void export_parameters(void);
//##############################################################################
// Handle parameters (GET PARAMETER FUNCTIONS)
//##############################################################################
// _____________________________________________________________________________
// DOUBLE:
// _____________________________________________________________________________
  /** \brief Returns the double value of the requested parameter
   *  using the function get_double(std::string).
   */
  double get_double(const char *parameter);
// _____________________________________________________________________________
  /** \brief Returns the double value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \return the value text of parameter interpreted as a double.
   *
   *  Searches the vector parameters for parameter. If not found, a
   *  BadParamException is thrown. If the parameter is defined, its value text
   *  is interpreted as a double.
   */
  double get_double(std::string parameter);
// _____________________________________________________________________________
  /** \brief Returns the double value of the requested parameter
   *  using the function get_double(std::string,double).
   */
  double get_double(const char *parameter, double standard);
// _____________________________________________________________________________
  /** \brief Returns the double value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \param standard Value that is returned, if parameter is not defined.
   *  \return the value text of parameter interpreted as a double or the
   *          standard value.
   *
   *  Searches the vector parameters for parameter. If not found, the standard
   *  value is returned. If the parameter is defined, its value text
   *  is interpreted as a double.
   */
  double get_double(std::string parameter, double standard);
// _____________________________________________________________________________
// INT:
// _____________________________________________________________________________
  /** \brief Returns the int value of the requested parameter
   *  using the function get_int(std::string).
   */
  int get_int(const char *parameter);
// _____________________________________________________________________________
  /** \brief Returns the int value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \return the value text of parameter interpreted as an int.
   *
   *  Searches the vector parameters for parameter. If not found, a
   *  BadParamException is thrown. If the parameter is defined, its value text
   *  is interpreted as an int.
   */
  int get_int(std::string parameter);
// _____________________________________________________________________________
  /** \brief Returns the int value of the requested parameter
   *  using the function get_int(std::string,int).
   */
  int get_int(const char *parameter, int standard);
// _____________________________________________________________________________
  /** \brief Returns the int value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \param standard Value that is returned, if parameter is not defined.
   *  \return the value text of parameter interpreted as an int or the
   *          standard value.
   *
   *  Searches the vector parameters for parameter. If not found, the standard
   *  value is returned. If the parameter is defined, its value text
   *  is interpreted as an int.
   */
  int get_int(std::string parameter, int standard);
// _____________________________________________________________________________
// BOOL:
// _____________________________________________________________________________
  /** \brief Returns the bool value of the requested parameter
   *  using the function get_bool(std::string).
   */
  bool get_bool(const char *parameter);
// _____________________________________________________________________________
  /** \brief Returns the bool value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \return the value text of parameter interpreted as a bool.
   *
   *  Searches the vector parameters for parameter. If not found, a
   *  BadParamException is thrown. If the parameter is defined, its value text
   *  is interpreted as a bool.
   *
   *  The value text can be ("false","False","FALSE","0") for false and
   *  ("true","True","TRUE","1") for true. If the parameter is set and the
   *  value text has non of this values, a BadParamException is thrown.
   */
  bool get_bool(std::string parameter);
// _____________________________________________________________________________
  /** \brief Returns the bool value of the requested parameter
   *  using the function get_bool(std::string,bool).
   */
  bool get_bool(const char *parameter, bool standard);
// _____________________________________________________________________________
  /** \brief Returns the bool value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \param standard Value that is returned, if parameter is not defined.
   *  \return the value text of parameter interpreted as a bool or the
   *          standard value.
   *
   *  Searches the vector parameters for parameter. If not found, the standard
   *  value is returned. If the parameter is defined, its value text
   *  is interpreted as a bool.
   *
   *  The value text can be ("false","False","FALSE","0") for false and
   *  ("true","True","TRUE","1") for true. If the parameter is set and the
   *  value text has non of this values, a BadParamException is thrown.
   */
  bool get_bool(std::string parameter, bool standard);
// _____________________________________________________________________________
// CHAR*:
// _____________________________________________________________________________
  /** \brief Returns the char* value of the requested parameter
   *  using the function get_char(std::string).
   *
   *  \note This function allocates memory for setting up the return C string.
   */
  char* get_char(const char *parameter);
// _____________________________________________________________________________
  /** \brief Returns the char* value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \return the value text of parameter interpreted as a char*.
   *
   *  Searches the vector parameters for parameter. If not found, a
   *  BadParamException is thrown. If the parameter is defined, its value text
   *  is interpreted as a char*.
   *
   *  \note This function allocates memory for setting up the return C string.
   */
  char* get_char(std::string parameter);
// _____________________________________________________________________________
  /** \brief Returns the char* value of the requested parameter
   *  using the function get_char(std::string,char*).
   *
   *  \note This function might allocates memory for setting up the return
   *  C string.
   */
  char* get_char(const char *parameter, char *standard);
// _____________________________________________________________________________
  /** \brief Returns the char* value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \param standard Value that is returned, if parameter is not defined.
   *  \return the value text of parameter interpreted as a char* or the
   *          standard value.
   *
   *  Searches the vector parameters for parameter. If not found, the standard
   *  value is returned. If the parameter is defined, its value text
   *  is interpreted as a char*.
   *
   *  \note This function allocates memory for setting up the return C string,
   *        if the parameter is defined.
   */
  char* get_char(std::string parameter, char *standard);
// _____________________________________________________________________________
// STD::STRING:
// _____________________________________________________________________________
  /** \brief Returns the std::string value of the requested parameter
   *  using the function get_string(std::string).
   */
  std::string get_string(const char *parameter);
// _____________________________________________________________________________
  /** \brief Returns the std::string value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \return the value text of parameter interpreted as a std::string.
   *
   *  Searches the vector parameters for parameter. If not found, a
   *  BadParamException is thrown. If the parameter is defined, its value text
   *  is interpreted as a std::string.
   */
  std::string get_string(std::string parameter);
// _____________________________________________________________________________
  /** \brief Returns the std::string value of the requested parameter
   *  using the function get_string(std::string,std::string).
   */
  std::string get_string(const char *parameter, std::string standard);
// _____________________________________________________________________________
  /** \brief Returns the std::string value of the requested parameter.
   *
   *  \param parameter Name of the parameter, whos value is requested.
   *  \param standard Value that is returned, if parameter is not defined.
   *  \return the value text of parameter interpreted as a std::string or the
   *          standard value.
   *
   *  Searches the vector parameters for parameter. If not found, the standard
   *  value is returned. If the parameter is defined, its value text
   *  is interpreted as a std::string.
   */
  std::string get_string(std::string parameter, std::string standard);
// _____________________________________________________________________________
// STD::VECTOR<INT>: 
// _____________________________________________________________________________
  /** \brief Returns the std::vector<int> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \return the value text of parameter interpreted as a std::vector<int>.
   *
   *  Searches the vector parameters for parameter. If not found, a 
   *  BadParamException is thrown. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<int>. 
   */
  std::vector<int> get_vector_int(const char* parameter);
  /** \brief Returns the std::vector<int> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \return the value text of parameter interpreted as a std::vector<int>. 
   *
   *  Searches the vector parameters for parameter. If not found, a 
   *  BadParamException is thrown. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<int>. 
   */
  std::vector<int> get_vector_int(std::string parameter);
  /** \brief Returns the std::vector<int> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \param standard Value that is returned, if parameter is not defined. 
   *  \return the value text of parameter interpreted as a std::vector<int> or
   *          the standard value. 
   *
   *  Searches the vector parameters for parameter. If not found, the standard 
   *  value is returned. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<int>. 
   */
  std::vector<int> get_vector_int(
      const char* parameter, std::vector<int> standard
  );
  /** \brief Returns the std::vector<int> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \param standard Value that is returned, if parameter is not defined. 
   *  \return the value text of parameter interpreted as a std::vector<int> or
   *          the standard value. 
   *
   *  Searches the vector parameters for parameter. If not found, the standard 
   *  value is returned. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<int>. 
   */
  std::vector<int> get_vector_int(
      std::string parameter, std::vector<int> standard
  );
// _____________________________________________________________________________
// STD::VECTOR<DOUBLE>: 
// _____________________________________________________________________________
  /** \brief Returns the std::vector<double> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \return the value text of parameter interpreted as a std::vector<double>.
   *
   *  Searches the vector parameters for parameter. If not found, a 
   *  BadParamException is thrown. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<double>. 
   */
  std::vector<double> get_vector_double(const char* parameter);
  /** \brief Returns the std::vector<double> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \return the value text of parameter interpreted as a std::vector<double>. 
   *
   *  Searches the vector parameters for parameter. If not found, a 
   *  BadParamException is thrown. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<double>. 
   */
  std::vector<double> get_vector_double(std::string parameter);
  /** \brief Returns the std::vector<double> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \param standard Value that is returned, if parameter is not defined. 
   *  \return the value text of parameter interpreted as a std::vector<double>
   *          or the standard value. 
   *
   *  Searches the vector parameters for parameter. If not found, the standard 
   *  value is returned. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<double>. 
   */
  std::vector<double> get_vector_double(
      const char* parameter, std::vector<double> standard
  );
  /** \brief Returns the std::vector<double> value of the requested parameter. 
   * 
   *  \param parameter Name of the parameter, whos value is requested. 
   *  \param standard Value that is returned, if parameter is not defined. 
   *  \return the value text of parameter interpreted as a std::vector<double>
   *          or the standard value. 
   *
   *  Searches the vector parameters for parameter. If not found, the standard 
   *  value is returned. If the parameter is defined, its value text 
   *  is interpreted as a std::vector<double>. 
   */
  std::vector<double> get_vector_double(
      std::string parameter, std::vector<double> standard
  );
//##############################################################################
// EXCEPTIONS:
//##############################################################################
// _____________________________________________________________________________
  /** \brief std::exception BadParamException, derived for the ParameterHandler.
   */
  class BadParamException : public std::exception {
   public:
    /** \brief Overwrite the exception information function what().
     *
     *  \return the text "Some requested parameter is not defined.".
     */
    virtual const char* what(void) const throw() {
      return "Some requested parameter is not defined.";
    }
  }
  /** \brief Exception BadParamException bad_param for the ParameterHandler.
   *
   *  The exception is thrown
   *  by the get_[data type](std::string) function,
   *  by the find_option(std::string,int,int) function,
   *  if the requested parameter is not defined in the vector parameters.
   */
  bad_param;
// _____________________________________________________________________________
  /** \brief std::exception BadOptionException, derived for the
   *  ParameterHandler.
   */
  class BadOptionException : public std::exception {
   public:
    /** \brief Overwrite the exception information function what().
     *
     *  \return the text "Some requested option is not defined.".
     */
    virtual const char* what(void) const throw() {
      return "Some requested option is not defined.";
    }
  }
  /** \brief Exception BadOptionException bad_option for the ParameterHandler.
   *
   *  The exception is thrown by the find_flag(int&) function, if a flag does
   *  not have the number of options that is defined by the
   *  register_flag(const char*,const char*,int) function.
   */
  bad_option;
// _____________________________________________________________________________
  /** \brief std::exception BadFileException, derived for the ParameterHandler.
   */
  class BadFileException : public std::exception {
   public:
    /** \brief Overwrite the exception information function what().
     *
     *  \return the text "A parameter file could not be handled.".
     */
    virtual const char* what(void) const throw() {
      return "A parameter file could not be handled.";
    }
  }
  /** \brief Exception BadFileException bad_file for the ParameterHandler.
   *
   *  The exception is thrown
   *  by the load_parameter_file(std::string) function, if the file name could
   *  not be handled.
   */
  bad_file;
//##############################################################################
// PRIVATE:
//##############################################################################
 private:
// _____________________________________________________________________________
  /** \brief Number of command line arguments (as passed to main).
   *
   *  Value is set by the Constructor ParameterHandler(int,char**).
   */
  int argc;
// _____________________________________________________________________________
  /** \brief Vector of command line arguments (as passed to main).
   *
   *  Value is set by the Constructor ParameterHandler(int,char**).
   */
  std::vector<std::string> args;
// _____________________________________________________________________________
  /** \brief Data format for a flag. */
  struct flag {
    /** \brief The short form of the flag, e.g. "-h", "-f", etc.. */
    std::string short_name;
    /** \brief The long form of the flag, e.g. "--help", "--file", etc.. */
    std::string long_name;
    /** \brief The number of options, i.e. arguments that follow the flag. */
    int no_options;
    /** \brief Description of the flag as a vector of std::string's. */
    std::vector<std::string> description;
  };
// _____________________________________________________________________________
  /** \brief Vector of registered flags of type flag. */
  std::vector<flag> flag_list;
// _____________________________________________________________________________
  /** \brief Vector of usage text lines.
   *
   *  See add_usage(std::string) and show_usage(void) for more details.
   */
  std::vector<std::string> usage_text;
// _____________________________________________________________________________
  /** \brief Vector of parameter-value pairs.
   *
   *  The first std::string of each pair holds the parameter name,
   *  the second std::string holds the value of the parameter as text.
   *
   *  The parameters are set by the process_parameters(void) function.
   */
  std::vector<std::pair<std::string, std::string>> parameters;
// _____________________________________________________________________________
  /** \brief Vector of usage description text lines.
   *
   *  See add_description(std::string) and show_usage(void) for more details.
   */
  std::vector<std::string> usage_descriptions;
// _____________________________________________________________________________
  /** \brief Initializes the usage text with the program name.
   *
   *  Adds a text line to the usage_text vector that contains:
   *  - the string " Usage: ",
   *  - the program name as stored in the args[0] variable,
   *  - the string " [<flag> <opt1 ...> <flag> <opt1 ...>]".
   */
  void init_usage(void);
// _____________________________________________________________________________
  /** \brief Returns true, if the parameter is defined.
   *
   *  \param parameter Parameter name that is looked up in the parameters
   *                   vector.
   *  \return true, if the parameter is found in the parameters vector.
   *
   *  If a (param,value)-pair is defined in the vector parameters with
   *  the param==parameter, true is returned; false otherwise.
   */
  bool isset_parameter(std::string parameter);
// _____________________________________________________________________________
  /** \brief Finds the value that corresponds to a given parameter.
   *
   *  \param parameter Name of the parameter thats value will be returned.
   *  \return value text of the given parameter.
   *
   *  If parameter is not found in the parameters vector, a BadParamException
   *  bad_param is thrown.
   */
  std::string get_value(std::string parameter);
// _____________________________________________________________________________
  /** \brief Search vector parameters for a given parameter name.
   *
   *  \param parameter Name of the parameter thats position will be returned.
   *  \return position index in the parameters vector. If the parameter name
   *          is not found, -1 is returned.
   */
  int find_parameter_position(const std::string parameter);
// _____________________________________________________________________________
  /** \brief Adds all parameters defined in a parameter file specified by the
   *         flag -f.
   *
   *   If the standard flag (short_name "-f", long_name "--file") is set,
   *   its first option is interpreted as the file name of a parameter file.
   *   From this file, all (parameter name,value) pairs are loaded using the
   *   load_parameter_file(std::string) function.
   */
  void add_parameters_from_file(void);
// _____________________________________________________________________________
  /** \brief Load parameter-value pairs from a given file.
   *
   *  \param filename File name of the parameter file.
   *
   *  Opens the file given by filename and loads all (parameter name,value)
   *  pairs from the file into the parameters vector. If parameter names occur
   *  more than once, only the first occurence is loaded and repeating
   *  versions are ignored.
   *  All "#" signs and following text until the corresponing end of the line
   *  are interpreted as comments and are ignored.
   */
  void load_parameter_file(std::string filename);
// _____________________________________________________________________________
  /** \brief Adds all parameters defined via -p flags in the command line.
   *
   *  Adds all (parameter name,value) pairs to the parameters vector that are
   *  defined via the first option of a standard flag
   *  (short_name "-p", long_name "--param").
   *  The form is "... -p [parameter name]=[value text] ...".
   *  If a parameter name already exists in the parameters vector, its value
   *  text is updated by the newly found one.
   */
  void add_parameters_from_cmdline(void);
// _____________________________________________________________________________
  /** \brief Converts an unsigned to a signed int.
   *
   *  \param input unsigned int that will be converted.
   *  \return the converted input as an int.
   *
   *  If the input value is larger than the maximal (signed) integer value,
   *  an exception is thrown.
   */
  int unsigned_to_signed_int(unsigned int input);
// _____________________________________________________________________________
  /** \brief Converts strings of comma delimitted numbers (without spaces) into
   *         a vector of integers.
   * 
   *  \param str std::string to be converted.
   *  \return the converted input as vector<int>
   *
   *  This function converts strings containings sequences of numbers, which are
   *  delimited by commas, into a vector of integers (e.g. "0,5,-3" -->
   *  vector<int>{0, 5, -3}).
   *
   */
  std::vector<int> string_to_vector_int(std::string str);
// _____________________________________________________________________________
  /** \brief Converts strings of comma delimitted numbers (without spaces) into
   *         a vector of doubles.
   * 
   *  \param str std::string to be converted.
   *  \return the converted input as vector<double>
   *
   *  This function converts strings containings sequences of numbers, which are
   *  delimited by commas, into a vector of doubles (e.g. "0.1,5.5,-3.14" -->
   *  vector<int>{0.1, 5.5, -3.14}).
   *
   */
  std::vector<double> string_to_vector_double(std::string str);
};
#endif  // SRC_PARAMETER_HANDLER_HPP_
