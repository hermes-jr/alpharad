#!/usr/bin/env python3
"""
Read text file with "X:Y" coordinates in each line.
Read binary file with random sequence.

Build scatter plots along with distribution of pixel hit frequencies
for x and y coordinates.
Build distribution of byte frequencies plot.

Introduces some errors caused by vector to raster conversion.
Sadly, I couldn't overcome this but nvm, this is just a hobby project =)
"""
try:
    from scipy import stats
except ImportError:
    print("scipy.stats not imported, skipping data stats calculation")
    stats = None

import matplotlib.pyplot as plt

xs, ys, bytes = [], [], []
title = 'alpharad plot'


def read_data():
    global title
    with open('../points.log', 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coords = line.split(':')
            xs.append(int(coords[0]))
            ys.append(int(coords[1]))
    print("Points read: {}".format(len(xs)))

    with open('../out.dat', 'rb') as f:
        byte = f.read(1)
        while byte:
            bytes.append(int.from_bytes(byte, "big", signed=False))
            byte = f.read(1)
    print("Bytes read: {}".format(len(bytes)))

    title = "{} flashes {} bytes".format(len(xs), len(bytes))


def plot():
    fig = plt.figure()
    fig.set_size_inches(12, 9)
    fig.canvas.set_window_title(title)
    plt.suptitle(title)

    gs0 = fig.add_gridspec(1, 2)
    gs00 = gs0[0].subgridspec(2, 1)
    gs01 = gs0[1].subgridspec(3, 1)

    fig.add_subplot(gs00[0, 0])
    plt.title("Hits - big markers")
    plt.scatter(xs, ys, marker='.', alpha=0.33, edgecolors='none')

    fig.add_subplot(gs00[1, 0])
    plt.title("Hits - small markers")
    plt.scatter(xs, ys, marker=',', s=1, alpha=0.33, edgecolors='none', snap=False, lw=1)

    fig.add_subplot(gs01[0, 0])
    plt.title("Xs distribution")
    plt.hist(xs, bins=640, snap=False, aa=False)

    fig.add_subplot(gs01[1, 0])
    plt.title("Ys distribution")
    plt.hist(ys, bins=480, snap=False, aa=False)

    fig.add_subplot(gs01[2, 0])
    plt.title("Bytes distribution")
    plt.hist(bytes, bins=256, snap=False, aa=False)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])

    print("Opening plot window ")
    #plt.show()
    plt.savefig('plot_{:08d}.png'.format(len(bytes)))


if __name__ == "__main__":
    read_data()

#    if stats:
#        print("x stats: {}\ny stats: {}".format(
#            stats.describe(xs),
#            stats.describe(ys)))
#        print("byte stats: {}".format(stats.describe(bytes)))

    plot()
