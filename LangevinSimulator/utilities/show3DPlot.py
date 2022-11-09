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

parser = argparse.ArgumentParser()

parser.add_argument(
    "-f",
    "--filename",
    help="Filename of data.",
    type=str)
parser.add_argument(
    "-c",
    "--clip",
    default=0,
    help="Number of values at edges that shall be ignored.",
    type=int)
parser.add_argument(
    "-o",
    "--observable",
    default=0,
    help="Index of observable to plot.",
    type=int)

args = parser.parse_args()

t1, t2, dat = loadFile(args.filename)

dat = dat.transpose((2,0,1))[args.observable]


def removeEdges(arr, width):
    arr = arr.T[width:-width]
    return arr.T[width:-width]


t1 = removeEdges(t1, args.clip)
t2 = removeEdges(t2, args.clip)
dat = removeEdges(dat, args.clip)

fig = plt.figure()
ax = fig.add_subplot(111, projection="3d")
ax.plot_surface(t1, t2, dat, cmap=cm.coolwarm)
#ax.set_zlim(-0.2, 1.2)
#plt.show()
ax.view_init(elev=-135, azim=-5)
plt.savefig("3d_plot.png")
