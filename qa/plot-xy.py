#!/usr/bin/env python3
try:
    from scipy import stats
except ImportError:
    print("scipy.stats not imported, skipping data stats calculation")
    stats = None

import matplotlib.pyplot as plt

xs, ys = [], []
title = 'alpharad plot'


def read_data():
    global title
    with open('points.txt', 'r') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            coords = line.split(':')
            xs.append(int(coords[0]))
            ys.append(int(coords[1]))
    print("Points read: {}".format(len(xs)))
    title = "{} flashes processed".format(len(xs))


def plot():
    fig = plt.figure()
    fig.set_size_inches(11, 8)
    fig.canvas.set_window_title(title)
    plt.suptitle(title)

    plt.subplot(2, 2, 1)
    plt.title("Hits - big markers")
    plt.scatter(xs, ys, marker='.', alpha=0.33, edgecolors='none')

    plt.subplot(2, 2, 2)
    plt.title("Hits - small markers")
    plt.scatter(xs, ys, marker=',', s=1, alpha=0.33, edgecolors='none')

    plt.subplot(2, 2, 3)
    plt.title("Xs distribution")
    plt.hist(xs, bins=640)

    plt.subplot(2, 2, 4)
    plt.title("Ys distribution")
    plt.hist(ys, bins=480)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])

    print("Opening plot window ")
    plt.show()


if __name__ == "__main__":
    read_data()

    if stats:
        print("x stats: {}\ny stats: {}".format(stats.describe(xs), stats.describe(ys)))

    plot()
