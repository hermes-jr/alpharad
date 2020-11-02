#!/usr/bin/env python3
"""
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

alpharad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with alpharad.  If not, see <https://www.gnu.org/licenses/>.
"""

"""
Read text file with "X:Y" coordinates in each line.

Build a grayscale image (in grades, up to 3 hits per pixel with more
hits marked as just white)

Can be ran as cron job to take occasional snapshots.
"""
import argparse

import numpy as np
from PIL import Image

settings = None
data = None
numpoints = 0


def init_arg_parser():
    ap = argparse.ArgumentParser(
        description='Reads points log file draws scatter plots')
    ap.add_argument('-p', '--points', default='../points.log', metavar='POINTS_LOG',
                    type=argparse.FileType('r'),
                    help='read hit coordinates from POINTS_LOG')
    ap.add_argument('--width', type=int, default=640,
                    help='plot width')
    ap.add_argument('--height', type=int, default=480,
                    help='plot height')
    return ap


def plot():
    img = Image.fromarray(data)
    img.save('vis_gray_{:08d}_hits.png'.format(numpoints))

    # Transform data, each pixel becomes either hit or not
    it = np.nditer(data, flags=['multi_index'], op_flags=['readwrite'])
    while not it.finished:
        data[it.multi_index] = 0 if data[it.multi_index] == 0 else 255
        it.iternext()

    img = Image.fromarray(data)
    img.save('vis_bw_{:08d}_hits.png'.format(numpoints))


def read_data():
    global numpoints
    global data
    data = np.zeros((settings.height, settings.width, 3), dtype=np.uint8)
    with settings.points as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coords = line.split(':')
            x = int(coords[0])
            y = int(coords[1])
            # Grayscale, 0 + 3 grades
            data[y, x] = [min(component + 85, 255) for component in data[y, x]]
            numpoints += 1


if __name__ == "__main__":
    settings = init_arg_parser().parse_args()
    read_data()
    plot()
