#!/usr/bin/env python3
"""
Read text file with "X:Y" coordinates in each line.
Read binary file with random sequence.

Build scatter plots along with distribution of pixel hit frequencies
for x and y coordinates.
Build distribution of byte frequencies plot.
"""
import argparse

from matplotlib.ticker import FormatStrFormatter

try:
    from scipy import stats
except ImportError:
    print("scipy.stats not imported, skipping data stats calculation")
    stats = None

import matplotlib.pyplot as plt

settings = None
xs, ys, as_bytes = [], [], []
title = 'alpharad plot'
colors = ['blue', 'green', 'red', 'cyan', 'magenta', 'yellow']


def v_print(param):
    if settings.verbose:
        print(param)


def read_data():
    global title
    with settings.points as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coords = line.split(':')
            xs.append(int(coords[0]))
            ys.append(int(coords[1]))
    v_print("Points read: {}".format(len(xs)))

    with settings.data as f:
        byte = f.read(1)
        while byte:
            as_bytes.append(int.from_bytes(byte, "big", signed=False))
            byte = f.read(1)

    v_print("Bytes read: {}".format(len(as_bytes)))

    title = "{} flashes {} bytes".format(len(xs), len(as_bytes))


def init_arg_parser():
    ap = argparse.ArgumentParser(
        description='Reads points log file and a binary data associated to it and draws numbers distribution plots')
    ap.add_argument('-d', '--data', default='../out.dat',
                    type=argparse.FileType('rb'),
                    help='read generated sequence of bytes from DATA')
    ap.add_argument('-p', '--points', default='../points.log', metavar='POINTS_LOG',
                    type=argparse.FileType('r'),
                    help='read hit coordinates from POINTS_LOG')
    ap.add_argument('--stats', action='store_true',
                    help='calculate and display data statistics after processing')
    ap.add_argument('--window', action='store_true',
                    help='display plot window. Default: write output to file')
    ap.add_argument('-v', '--verbose', action='store_true',
                    help='explain what is being done')
    return ap


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
    plt.scatter(xs, ys, marker='.', alpha=0.33, edgecolors='none',
                c=(tuple(colors[i % len(colors)] for i in range(len(xs))))
                )

    fig.add_subplot(gs00[1, 0])
    plt.title("Hits - small markers")
    plt.scatter(xs, ys, marker=',', s=1, alpha=0.33, edgecolors='none', snap=False, lw=1)

    fig.add_subplot(gs01[0, 0])
    plt.title("Xs distribution")
    plt.hist(xs, bins=640, snap=False, aa=False)
    plt.gca().yaxis.set_major_formatter(FormatStrFormatter('%6d'))

    fig.add_subplot(gs01[1, 0])
    plt.title("Ys distribution")
    plt.hist(ys, bins=480, snap=False, aa=False)
    plt.gca().yaxis.set_major_formatter(FormatStrFormatter('%6d'))

    fig.add_subplot(gs01[2, 0])
    plt.title("Bytes distribution")
    plt.hist(as_bytes, bins=256, snap=False, aa=False)
    plt.gca().yaxis.set_major_formatter(FormatStrFormatter('%6d'))

    plt.tight_layout()

    if settings.window:
        v_print("Opening plot window")
        plt.show()
    else:
        plt.savefig('plot_{:08d}.png'.format(len(as_bytes)))


if __name__ == "__main__":
    settings = init_arg_parser().parse_args()

    read_data()

    if stats and settings.stats:
        print("x stats: {}\ny stats: {}".format(
            stats.describe(xs),
            stats.describe(ys)))
        print("byte stats: {}".format(stats.describe(as_bytes)))

    plot()
