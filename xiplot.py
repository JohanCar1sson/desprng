import array
import numpy as np
import matplotlib.pyplot as plt
import sys

Nbin = 40
dxi = 2.0 / Nbin
xi = np.linspace(0.5 * dxi - 1.0, 1.0 - 0.5 * dxi, num=Nbin)

plt.rcParams.update({'font.size': 16})

unsigned_long_data = array.array('L')
double_data = array.array('d')

xidump = open('xi.dat', 'rb')

unsigned_long_data.fromfile(xidump, 1)
Npart = unsigned_long_data[0]
print('Npart = ', Npart)

double_data.fromfile(xidump, 1)
xt = double_data[0]
print('xt = ', xt)

double_data.fromfile(xidump, Npart)
xidata = np.array(double_data)
#print(xidata)

hist, bins = np.histogram(xidata, Nbin, range=(-1.0, 1.0))
f_of_xi = np.array(hist) / (dxi * Npart)

plt.plot(xi, f_of_xi)
plt.xlim([-1.0, 1.0])
plt.ylabel(r'$f \, (\xi)$')
plt.xlabel(r'$\xi$')
plt.tight_layout()
plt.savefig('xi.png', format = 'png')
plt.close()

xidump.close()
