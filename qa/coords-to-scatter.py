#!/usr/bin/env python3
"""
Read text file with "X:Y" coordinates in each line.

Build a grayscale image (in grades, up to 3 hits per pixel with more
hits marked as just white)

Can be ran as cron job to take occasional snapshots.
"""
import numpy as np
from PIL import Image

# Hardcoded to debug borderline pixels behavior and other possible artifacts
width = 640
height = 480

data = np.zeros((height, width, 3), dtype=np.uint8)
numpoints = 0

with open('../points.log', 'r') as f:
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

img = Image.fromarray(data)
img.save('vis_gray_{:08d}_hits.png'.format(numpoints))

# Transform data, each pixel becomes either hit or not
it = np.nditer(data, flags=['multi_index'], op_flags=['readwrite'])
while not it.finished:
    data[it.multi_index] = 0 if data[it.multi_index] == 0 else 255
    it.iternext()

img = Image.fromarray(data)
img.save('vis_bw_{:08d}_hits.png'.format(numpoints))
