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

args = parser.parse_args()

t1, t2, dat = loadFile(args.filename)
num_obs = int(len(dat[0][0])**.5)


def removeEdges(arr, width):
    tr_idx = list(range(len(arr.shape)))
    tr_idx[0] = 1
    tr_idx[1] = 0
    arr = arr.transpose(tr_idx)[width:-width]
    return arr.transpose(tr_idx)[width:-width]


if args.clip != 0:
    t1 = removeEdges(t1, args.clip)
    t2 = removeEdges(t2, args.clip)
    dat = removeEdges(dat, args.clip)
dat = dat.transpose((2,0,1))


fig,axs = plt.subplots(num_obs, num_obs)

if num_obs != 1:
    for i in range(num_obs):
        for j in range(num_obs):
            #axs[i, j].imshow(
            #    dat[i * num_obs + j],
            #    origin="lower",
            #    extent=[t2[0][0], t2[-1][-1], t1[0][0], t1[-1][-1]])
            c = axs[i, j].pcolormesh(t2, t1, dat[i*num_obs+j])
            axs[i, j].invert_yaxis()
            plt.colorbar(c, ax=axs[i, j])
            axs[i, j].set_xlabel("$t_2$")
            axs[i, j].set_ylabel("$t_1$")
            axs[i, j].set_title("Observables: $A_{{{:d}}} A_{{{:d}}}$".format(i+1, j+1))
else:
    c = axs.pcolormesh(t2, t1, dat[0])
    axs.invert_yaxis()
    plt.colorbar(c, ax=axs)
    axs.set_xlabel("$t_2$")
    axs.set_ylabel("$t_1$")
    axs.set_title("Observables: $A_{{{:d}}} A_{{{:d}}}$".format(1, 1))
  

plt.tight_layout()
#plt.show()
plt.savefig("heatmap.png")
