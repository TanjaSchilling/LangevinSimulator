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

import struct
import numpy as np

def readBinary(filename):
  with open(filename, "rb") as file_handler:
    header_size = struct.unpack('n', file_handler.read(8))[0]
    shape = struct.unpack(header_size * 'n', file_handler.read(header_size * 8))
    data_size = struct.unpack('n', file_handler.read(8))[0]
    data = np.zeros(data_size, dtype=np.double)
    for i in range(data_size):
      data[i] = struct.unpack('d', file_handler.read(8))[0]
    data = data.reshape(shape)
    return data

def writeBinary(data, filename):
  with open(filename, "wb") as file_handler:
    file_handler.write(struct.pack('n', len(data.shape)))
    for dim in data.shape:
      file_handler.write(struct.pack('n', dim))
    data_size = np.prod(data.shape)
    file_handler.write(struct.pack('n', data_size))
    x = data.flatten()
    for i in range(data_size):
      file_handler.write(struct.pack('d', x[i]))
      
traj = readBinary("traj.f64")
times = readBinary("times.f64")

import matplotlib.pyplot as plt
plt.plot(times,traj[0,:,0],label="first trajectory")
plt.plot(times,traj[1,:,0],label="second trajectory")
plt.legend()
plt.show()





