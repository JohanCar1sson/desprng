import array
import numpy as np
import matplotlib.pyplot as plt
from scipy.special import legendre
import math

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

double_data.fromfile(xidump, 2)
xi0 = double_data[0]
xt = double_data[1]
print('xi0 = ', xi0, ', xt = ', xt)

double_data.fromfile(xidump, Npart)
xidata = np.array(double_data)
#print(xidata)

hist, bins = np.histogram(xidata, Nbin, range=(-1.0, 1.0))
f_of_xi = np.array(hist) / (dxi * Npart)

fa = np.zeros(Nbin)
for l in range(12):
  func = legendre(l)
  for m in range(Nbin):
    fa[m] += (l + 0.5) * func(xi0) * func(xi[m]) * math.exp(-l * (l + 1) * xt)

plt.plot(xi, fa, 'r', label = 'analytic solution')
plt.plot(xi, f_of_xi, 'b', label = 'PIC-MCC solution')
plt.xlim([-1.0, 1.0])
plt.ylabel(r'$f \, (\xi)$')
plt.xlabel(r'$\xi$')
plt.legend()
plt.tight_layout()
plt.savefig('xi.png', format = 'png')
plt.close()

xidump.close()
