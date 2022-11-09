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

import numpy as np
from functools import reduce


def loadFile(filename):
  t1, t2, dat = [], [], []
  with open(filename, "r") as f:
    for line in f:
      if line == "" or line == "\n":
        continue
      entries = line.strip().split()
      entries = [float(e) for e in entries]
      t1.append(entries.pop(0))
      t2.append(entries.pop(0))
      dat.append(entries[:])
  observables = len(entries)
  dim = t1.count(t1[0])
  t1 = np.array(t1)
  t1 = t1.reshape((dim, dim))
  t2 = np.array(t2)
  t2 = t2.reshape((dim, dim))
  dat = np.array(dat)
  dat = dat.reshape((dim, dim, observables))
  return t1, t2, dat


def writeFile(t1, t2, dat, filename):
  dim = len(t1)
  t1 = t1.reshape((dim**2,))
  t2 = t2.reshape((dim**2,))
  try:
    observables = reduce(lambda a,b: a*b, dat.shape[2:])
  except:
    observables = 1
  dat = dat.reshape((dim**2, observables))
  with open(filename, "w") as f:
    for i, (t1_n, t2_n, dat_n) in enumerate(zip(t1, t2, dat)):
      f.write(
        ("{:f}\t{:f}" + observables * "\t{:10e}" + "\n").format(
          t1_n, t2_n, *dat_n
          )
        )
      i += 1
      if i % dim == 0:
        f.write("\n")
  print(filename, "written")
