#!/usr/bin/env python3
try:
    from scipy import stats
except ImportError:
    print("scipy.stats not imported, skipping data stats calculation")
    stats = None

import matplotlib.pyplot as plt

xs, ys = [], []


def read_data():
    with open('points.txt', 'r') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            coords = line.split(':')
            xs.append(int(coords[0]))
            ys.append(int(coords[1]))
    print("Points read: {}".format(len(xs)))


def plot():
    fig, ax = plt.subplots()
    ax.scatter(xs, ys, marker='.', alpha=0.33, edgecolors='none')
    #ax.scatter(xs, ys, marker=',', s=1, alpha=0.33, edgecolors='none')

    # DPI = fig.get_dpi()
    # fig.set_size_inches(640.0/float(DPI),480.0/float(DPI))
    # ax.set_xlim(0, 640)
    # ax.set_ylim(0, 480)
    print("Opening plot window ")
    plt.show()


if __name__ == "__main__":
    read_data()

    if stats:
        print("x stats: {}\ny stats: {}".format(stats.describe(xs), stats.describe(ys)))

    plot()
