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

#!/bin/python3

import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pylab as plt
from matplotlib import cm
import sys, argparse
from scipy.optimize import curve_fit

from inputOutputHandler import *


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ("yes", "true", "t", "y", "1"):
        return True
    elif v.lower() in ("no", "false", "f", "n", "0"):
        return False
    else:
        raise argparse.ArgumentTypeError("Boolean value expected.")

parser = argparse.ArgumentParser()

parser.add_argument(
    "-f",
    "--filename",
    help="Filename of data.",
    type=str)
parser.add_argument(
    "-t",
    "--times",
    help="Times t1 for the plotted kernels.",
    type=eval)
parser.add_argument(
    "-o",
    "--observable",
    default=0,
    help="Index of observable to plot.",
    type=int)
parser.add_argument(
    "-n",
    "--normalize",
    default=True,
    help="Normalize with value at t1=t2",
    type=str2bool)
parser.add_argument(
    "-e",
    "--errors",
    default="",
    help="File containing standard deviation.",
    type=str)

args = parser.parse_args()
t1, t2, dat = loadFile(args.filename)

dat = dat.transpose((2,0,1))[args.observable]


indx = []
for t in args.times:
    indx.append(np.abs(t2[0] - t).argmin())

if args.errors:
    a, b, err = loadFile(args.errors)
    err = err.transpose((2,0,1))[args.observable]
    del(a)
    del(b)

# plt settings
plt.rcParams.update({'figure.figsize': [19.2, 10.8]})
plt.rcParams.update({'font.size': 22})
plt.rcParams.update({'legend.framealpha': 0.5})
prop_cycle = plt.rcParams['axes.prop_cycle']
colors = prop_cycle.by_key()['color']

if args.normalize:
    for j, i in enumerate(indx):
        color = colors[j]
        plt.plot(t2[i][i]-t2[i], dat[i]/dat[i][i], label = str(t2[i][i]), color=color)
        if args.errors:
            # Be cautious! No correct error propagation is done here!
            plt.plot(t2[i][i]-t2[i], (dat[i]-err[i])/dat[i][i], color=color, lw=.5)
            plt.plot(t2[i][i]-t2[i], (dat[i]+err[i])/dat[i][i], color=color, lw=.5)
    plt.ylabel("$K(t_1,t_2)/K(t_1,t_1)$")
else:
    for j, i in enumerate(indx):
        color = colors[j]
        plt.plot(t2[i][i]-t2[i], dat[i], label = str(t2[i][i]), color=color)
        if args.errors:
            # Be cautious! No correct error propagation is done here!
            plt.plot(t2[i][i]-t2[i], dat[i]-err[i], color=color, lw=.5)
            plt.plot(t2[i][i]-t2[i], dat[i]+err[i], color=color, lw=.5)
    plt.ylabel("$K(t_1,t_2)$")
plt.xlabel("$t_1-t_2$")
plt.grid()
plt.tight_layout()
plt.legend(loc="best", title="$t_1$")
plt.savefig("memory_kernel_obs_"+str(args.observable)+".png")
#plt.show()
