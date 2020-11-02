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

import hashlib
import math

import numpy as np

points = []
xqf_sim, yqf_sim = None, None

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


def m_compare_per_frame():
    """
    :return: 1 byte every 16 frames
    """
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


def m_compare_first_try():
    """
    The very first attempt
    :return:  1 byte every 8 frames
    """
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


def m_sha():
    """
    When we detect a flash, we feed the whole frame state to sha256 function. Thus, we guarantee that at least
    one pixel at truly random position is lit, which in turn means that the whole hash is unpredictable.
    Thermal noise of other dark pixels adds entropy.

    :return: 32 bytes per frame that contains at least one flash
    (for the purposes of this script it is safe to assume that each frame contains only a single flash)
    """
    global points
    hash_len = 32

    # One hash per point, limit to 1M of data to speedup
    result = np.empty(min(len(points) * hash_len, 1_000_000), dtype=np.dtype('b'))
    offset = 0
    for i in range(min(len(points), 1_000_000 // hash_len)):
        x, y = points[i]

        # Dark noisy pixels (thermal noise emulation)
        frame = np.random.randint(-128, -90, width * height * 2, np.dtype('b'))

        # Flash
        frame[(y * width + x) * 2] = np.random.randint(120, 127)

        h = hashlib.sha256(bytes(frame))
        np.put(result, range(offset, offset + hash_len), list(bytearray(h.digest())))
        offset += hash_len
    return result


def m_box_muller():
    """
    An attempt to do a probability integral transform / Box-Muller
    This is a playground. I hope no one ever sees this :)
    cuz I'm ashamed for not understanding this
    """
    x_orig_acc = []
    y_orig_acc = []
    result = []
    for x, y in points:
        # for x, y in np.random.normal(0.5, 0.35, size=(2_000_000, 2)):
        # if x < 0 or x > 1 or y < 0 or y > 1:
        #   continue
        # x_orig = x
        # y_orig = y

        x_orig = x / (width - 1)
        y_orig = y / (height - 1)
        x_orig_acc.append(x_orig)
        y_orig_acc.append(y_orig)
        x_orig_as_byte = int(x_orig * 256)
        y_orig_as_byte = int(y_orig * 256)
        if len(x_orig_acc) < 250:
            continue
        else:
            # nobs, mm, mean, variance, sk, kt = stats.describe(x_orig)
            # x_mu = mean
            # x_theta = np.sqrt(variance)
            # nobs, mm, mean, variance, sk, kt = stats.describe(x_orig)
            # y_mu = mean
            # y_theta = np.sqrt(variance)

            # y_orig = y_orig / y_theta - y_mu
            # x_orig = x_orig / x_theta - x_mu

            z1 = np.sqrt(-2 * np.log(x_orig)) * np.cos(2 * np.pi * y_orig)
            z2 = np.sqrt(-2 * np.log(x_orig)) * np.sin(2 * np.pi * y_orig)
            # z1 = x_orig
            # z2 = y_orig
            x_conv = np.exp((-1 / 2) * (z1 ** 2 + z2 ** 2))
            y_conv = (1 / np.pi) * np.arccos(z1 / np.sqrt(z1 ** 2 + z2 ** 2))
            x_conv_as_byte = int(x_conv * 256)
            y_conv_as_byte = int(y_conv * 256)
            # print('X: {} ({}) -> {} ({}) | Y: {} ({}) -> {} ({})'.format(x_orig, x_orig_as_byte,
            #                                                              x_conv, x_conv_as_byte,
            #                                                              y_orig, y_orig_as_byte,
            #                                                              y_conv, y_conv_as_byte))
            result.append(x_conv_as_byte)
            result.append(y_conv_as_byte)
    return result


def emulate_quantile_function(values, buckets):
    values_count = [0] * buckets
    result = [0.0] * buckets
    # Count frequencies for numbers in a sequence
    for pt in values:
        values_count[pt] += 1
    # For each value calculate the possibility of random number being lower or equal than this value
    for pv in range(buckets):
        result[pv] = sum(values_count[k] for k in range(pv + 1)) / len(values)
    return result


def m_quantile():
    xs_count = [0] * width
    ys_count = [0] * height
    xs_total, ys_total = 0, 0
    result = []
    sq = True
    for x, y in points:
        xs_total += 1
        xs_count[x] += 1

        ys_total += 1
        ys_count[y] += 1

        # Accumulate statistics data first
        if xs_total < 10_000:
            continue

        xfq = sum(xs_count[k] for k in range(x + 1)) / xs_total
        yfq = sum(ys_count[k] for k in range(y + 1)) / ys_total

        # Equalize the chances in byte conversion, eliminate smaller segments around 0.0 and 1.0
        # Simple flooring/ceiling would make some values to be 1.5 times frequent than others, RR helps
        if sq:
            x_conv = math.floor(257 * xfq) - 1
            y_conv = math.ceil(257 * yfq) - 1
        else:
            x_conv = math.ceil(257 * xfq) - 1
            y_conv = math.floor(257 * yfq) - 1
        sq ^= True

        # Discard less frequent values
        if 0 <= x_conv <= 255:
            result.append(x_conv)
        if 0 <= y_conv <= 255:
            result.append(y_conv)

    return result


def m_quantile_viktor():
    """
    Convert distribution to uniform, each coordinate spawns 4 bits

    Exchanging data quantity for quality

    :return: 1 byte per registered flash
    """
    result = []
    sq = True
    for x, y in points:
        x_chance = xqf_sim[x]
        y_chance = yqf_sim[y]

        # Spawn two halves of byte
        if sq:
            x_conv = math.floor(17 * x_chance) - 1
            y_conv = math.ceil(17 * y_chance) - 1
        else:
            x_conv = math.ceil(17 * x_chance) - 1
            y_conv = math.floor(17 * y_chance) - 1
        sq ^= True
        if 0 <= x_conv <= 15 and 0 <= y_conv <= 15:
            byte = x_conv << 4
            byte += y_conv
            result.append(byte)
    return result


def m_quantile_and_compare():
    """
    Same logic as in m_compare_first_try

    :return: 1 byte every 8 flashes
    """
    result = []
    buf_byte = 0
    bits = 0
    for x, y in points:
        x_chance = xqf_sim[x]
        y_chance = yqf_sim[y]

        buf_byte <<= 1
        if x_chance < y_chance:
            buf_byte |= 1
        bits += 1

        if bits == 8:
            result.append(buf_byte)
            buf_byte = 0
            bits = 0

    return result


def save_result(name, method):
    body = bytes(method())
    out_file = open('../out_{}.dat'.format(name), 'wb')
    out_file.write(body)
    out_file.close()


if __name__ == "__main__":
    get_data()

    xqf_sim = emulate_quantile_function(list(x for x, y in points), width)
    yqf_sim = emulate_quantile_function(list(y for x, y in points), height)

    methods = [
        m_compare_per_frame,
        m_compare_first_try,
        m_quantile,
        m_quantile_and_compare,
        m_quantile_viktor,
        # m_box_muller,
        m_sha,
    ]
    for v in methods:
        print("Calculating", v.__name__)
        save_result(v.__name__, v)
