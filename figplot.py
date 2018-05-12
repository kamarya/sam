#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

data = np.genfromtxt('results.csv',delimiter=',')
data = data[1:,:]

axes = plt.gca()
axes.set_xlim([data[-1,1],data[0,1]])
axes.set_ylim([-0.05,1.05])

plt.scatter(data[:,1],data[:,2], label='Guided Decod. 4 Iter.',marker='^')
plt.scatter(data[:,1],data[:,3], label='Blind Decod.',marker='s')
plt.xlabel('Number of Stored Messages')
plt.ylabel('Retrieval Error Rate')
plt.legend()
plt.grid()

plt.savefig('results.png')
plt.show()
