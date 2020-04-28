import numpy as np
from scipy import stats
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

"""
fig = plt.scatter(640, 480, marker='.' alpha=0.33)
xs = np.array([])
ys = np.array([])

ln, = ax.plot(xs,ys)


plt.plot(xs, ys, 'r.', alpha=0.25)
print(xs)
print(ys)

plt.show()
"""
xs, ys = [], []
with open('../stat.dat', 'r') as f:
    for line in f:
        line = line.strip()
        if(not line): continue
        coords = line.split(':')
        xs = np.append(xs, int(coords[0]));
        ys = np.append(ys, int(coords[1]));

print(stats.describe(xs))
print(stats.describe(ys))

fig, ax = plt.subplots()
ax.scatter(xs, ys, marker='.', alpha=0.33, edgecolors='none')

ax.set_xlim(0, 640)
ax.set_ylim(0, 480)

plt.show()
