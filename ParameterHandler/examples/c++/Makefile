# SPDX-FileCopyrightText: 2021 Andreas Härtel <http://andreashaertel.anno1982.de/>
# SPDX-License-Identifier: LGPL-3.0-or-later
# SPDX-PackageName: parameter_handler
# SPDX-PackageHomePage: https://github.com/andreashaertel/parameter_handler
################################################################################
#
# Makefile
# for the C++ examples in the project parameter_handler 
#
# The project contains the C++ library and python module parameter_handler. 
#
################################################################################
## binary name #################################################################
# Compiler #####################################################################
CXX = g++
# Include directories ##########################################################
INCDIRS = -I./ -I./include -I../../include
# Library directory ############################################################
LIBDIRS = -L./ -L./bin -L../../bin
# Libraries to be linked #######################################################
LIBS = -l$(HOST).parameter_handler
# Compiler flags ###############################################################
CXXFLAGS = -pedantic -std=c++2a -O2 -Wall $(INCDIRS)
# Linker flags #################################################################
LDFLAGS = $(LIBDIRS)
# Directories for binaries, objects and sources ################################
BDIR = bin/
ODIR = obj/
SDIR = src/
LOGDIR = log/
# Get host name ################################################################
HOST = $(shell hostname)
# Offsets for binaries and objects #############################################
BOFF = $(BDIR)$(HOST).
OOFF = $(ODIR)$(HOST).
LOGOFF = $(LOGDIR)$(HOST).
# Header files and object files ################################################
HEADERS = $(wildcard $(SDIR)*.hpp)
SOURCES = $(wildcard $(SDIR)*.cpp)
OBJECTS = $(patsubst $(SDIR)%.cpp,$(OOFF)%.o,$(SOURCES))
# System information files #####################################################
SYSINFO = $(LOGOFF)systeminfo
# TARGETS ######################################################################

.PHONY: default all init checkstyle compile info clean

default: info compile bind

all: checkstyle info compile bind

# Initialize
init:
	@echo " Make required directories ... "; mkdir -p obj; mkdir -p log; mkdir -p bin

# Google C++ style checker:
# Install cpplint with
# > $pip3 install cpplint
# and add ~/.local/bin to the global PATH variable
#	(for instance, by adding export PATH=$PATH:$HOME/.local/bin to your .bashrc file). 
# The style checker checks all C++ files in the src directory.
checkstyle:
ifeq (,$(shell echo $(PATH) | grep $(HOME)/.local/bin))
	$(error The path "$(HOME)/.local/bin" is not in your PATH variable; please add it)
endif
ifeq (,$(shell which cpplint))
	$(error cpplint is not installed; install it using "make cpplint")
endif
	@echo " Run Google C++ style checker on the source code ... "; cpplint src/*

cpplint: 
	pip3 install cpplint

# Write system information
info:
	@echo "HOSTNAME=$(shell hostname)" > $(SYSINFO)
	@echo "DATE=$(shell date)" >> $(SYSINFO)
	@echo "SYSTEM=$(shell uname -a)" >> $(SYSINFO)
	@echo "COMPILERVERSION=$(shell $(CXX) --version)" >> $(SYSINFO)
	@echo "CPUINFO=$(shell lshw -class processor 2>/dev/null)" >> $(SYSINFO)
	@echo "DISPLAY=$(shell lspci -vnn | grep "VGA" 2>/dev/null)" >> $(SYSINFO)

# Cleaning all targets (objects, binaries, and final includes)
clean:
	@echo " Cleaning objects ... "; rm -f $(ODIR)*
	@echo " Cleaning binaries ... "; rm -f $(BDIR)*
	@echo " Cleaning logs ... "; rm -f $(LOGDIR)*; rm -f params.log


################################################################################
# Binding
################################################################################

# Syntax: 
# $<	the first dependency
# $+	list of all dependencies
# $^	list of all dependencies; repeating entries are droped
# $@	name of the target

# Make programs
bind: $(BOFF)example $(BOFF)minimal_example

# Explicit rules to make programs
$(BOFF)example: $(OOFF)example.o
	@echo " Linking $@ ... "; $(CXX) -o $@ $(LIBDIRS) $^ $(LIBS)
$(BOFF)minimal_example: $(OOFF)minimal_example.o
	@echo " Linking $@ ... "; $(CXX) -o $@ $(LIBDIRS) $^ $(LIBS)

################################################################################
# Compiling
################################################################################

# Syntax: 
# $<	the first dependency
# $+	list of all dependencies
# $^	list of all dependencies; repeating entries are droped
# $@	name of the target

# Compile each source file (depending on its header file)
# 
# If each source only depends on its own header (no cross linking) than we 
# can use: 
# $(OOFF)%.o: $(SDIR)%.cpp $(SDIR)%.hpp
# 	@echo " Compiling $< ... "; $(CXX) $(CXXFLAGS) -c -o $@ $<
# However, sometimes we expect to find cross linking. 
# Then, we have to add an explicit rule. 


# Compile all objects and write system information
compile: info $(OBJECTS)


#################
# Explicit rules: 

$(OOFF)example.o: $(SDIR)example.cpp
	@echo " Compiling $< ... "; $(CXX) $(CXXFLAGS) -c -o $@ $<
$(OOFF)minimal_example.o: $(SDIR)minimal_example.cpp
	@echo " Compiling $< ... "; $(CXX) $(CXXFLAGS) -c -o $@ $<


