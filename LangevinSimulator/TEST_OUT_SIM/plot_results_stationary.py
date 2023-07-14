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
      
import matplotlib.pyplot as plt

times = readBinary("times.f64")

data=readBinary("../TEST_OUT/traj.f64")
plt.plot(times,data[0,:,0],color="black",label="original trajectory, first observable")
plt.plot(times,data[0,:,1],color="red",label="original trajectory, second observable")
data = readBinary("traj.f64")
plt.plot(times,data[0,:,0],color="blue",label="simulated trajectory, first observable")
plt.plot(times,data[0,:,1],color="green",label="simulated trajectory, second observable")
plt.legend()
plt.savefig("trajectories.png")
plt.show()

data=readBinary("../TEST_OUT/correlation_stationary.f64")
plt.plot(data[:,0,0],color="green",label="correlation function of original trajectories, observable 0x0")
data = readBinary("correlation_stationary.f64")
plt.plot(data[:,0,0],color="blue",label="correlation function of simulations, observable 0x0")
plt.legend()
plt.savefig("corr00.png")
plt.show()

data=readBinary("../TEST_OUT/correlation_stationary.f64")
plt.plot(data[:,1,1],color="green",label="correlation function of original trajectories, observable 1x1")
data = readBinary("correlation_stationary.f64")
plt.plot(data[:,1,1],color="blue",label="correlation function of simulations, observable 1x1")
plt.legend()
plt.savefig("corr11.png")
plt.show()

data=readBinary("../TEST_OUT/correlation_stationary.f64")
plt.plot(data[:,1,0],color="green",label="correlation function of original trajectories, observable 1x0")
data = readBinary("correlation_stationary.f64")
plt.plot(data[:,1,0],color="blue",label="correlation function of simulations, observable 1x0")
plt.legend()
plt.savefig("corr10.png")
plt.show()

data=readBinary("../TEST_OUT/correlation_stationary.f64")
plt.plot(data[:,0,1],color="green",label="correlation function of original trajectories, observable 0x1")
data = readBinary("correlation_stationary.f64")
plt.plot(data[:,0,1],color="blue",label="correlation function of simulations, observable 0x1")
plt.legend()
plt.savefig("corr01.png")
plt.show()

data=readBinary("../TEST_OUT/kernel_stationary.f64")
plt.plot(data[:,0,0],color="green",label="memory kernel of original trajectories, observable 0x0")
data = readBinary("kernel_stationary.f64")
plt.plot(data[:,0,0],color="blue",label="memory kernel of simulations, observable 0x0")
plt.legend()
plt.savefig("kernel00.png")
plt.show()

data=readBinary("../TEST_OUT/kernel_stationary.f64")
plt.plot(data[:,1,1],color="green",label="memory kernel of original trajectories, observable 1x1")
data = readBinary("kernel_stationary.f64")
plt.plot(data[:,1,1],color="blue",label="memory kernel of simulations, observable 1x1")
plt.legend()
plt.savefig("kernel11.png")
plt.show()

data=readBinary("../TEST_OUT/kernel_stationary.f64")
plt.plot(data[:,1,0],color="green",label="memory kernel of original trajectories, observable 1x0")
data = readBinary("kernel_stationary.f64")
plt.plot(data[:,1,0],color="blue",label="memory kernel of simulations, observable 1x0")
plt.legend()
plt.savefig("kernel10.png")
plt.show()

data=readBinary("../TEST_OUT/kernel_stationary.f64")
plt.plot(data[:,0,1],color="green",label="memory kernel of original trajectories, observable 0x1")
data = readBinary("kernel_stationary.f64")
plt.plot(data[:,0,1],color="blue",label="memory kernel of simulations, observable 0x1")
plt.legend()
plt.savefig("kernel01.png")
plt.show()



