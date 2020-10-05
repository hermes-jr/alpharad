#!/usr/bin/env python3
import hashlib

import numpy as np

points = []

width = 640
height = 480


def get_data():
    with open('../points.log', 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coords = line.split(':')
            points.append((int(coords[0]), int(coords[1])))
    print("Points read: {}".format(len(points)))


def mode_compare_per_frame():
    global points
    result = []
    buf_byte = 0
    bits = 0
    pv = -1

    for x, y in points:
        v2 = (x + y * width) / (width * height)
        if pv == -1:
            pv = v2
            continue
        else:
            v1 = pv
            pv = -1
        if v1 == v2:
            continue

        buf_byte <<= 1
        if v1 < v2:
            buf_byte |= 1
        bits += 1

        if bits == 8:
            result.append(buf_byte)
            buf_byte = 0
            bits = 0

    return result


def mode_compare_first_try():
    global points
    result = []
    buf_byte = 0
    bits = 0

    for x, y in points:
        v1 = (x - 1) / (width - 2)
        v2 = (y - 1) / (height - 2)

        buf_byte <<= 1
        if v1 < v2:
            buf_byte |= 1
        bits += 1

        if bits == 8:
            result.append(buf_byte)
            buf_byte = 0
            bits = 0

    return result


def mode_sha():
    global points
    result = np.empty(len(points) * 20, dtype=np.dtype('b'))  # one sha1 hash per point
    offset = 0
    for x, y in points:
        # Emulate thermal noise
        # frame = [random.randint(0, 60) for i in range(width * height * 2)]  # dark noisy pixels
        frame = np.random.rand(width * height * 2, 1)
        frame[(y * width + x) * 2] = np.random.randint(230, 255)  # flash
        h = hashlib.sha1(bytes(frame))
        np.put(result, range(offset, offset + 20), list(bytearray(h.digest())))
        offset += 20
    return result


def save_result(name, method):
    body = bytes(method())
    out_file = open('../out-{}.dat'.format(name), 'wb')
    out_file.write(body)
    out_file.close()


if __name__ == "__main__":
    get_data()

    methods = [
        # mode_compare_per_frame,
        # mode_compare_first_try,
        mode_sha
    ]
    for v in methods:
        save_result(v.__name__, v)
