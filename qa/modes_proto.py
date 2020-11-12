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
buf_byte = 0
bit_count = 0

width = 640
height = 480


def get_data():
    with open('../points.log', 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coordinates = line.split(':')
            points.append((int(coordinates[0]), int(coordinates[1])))
    print("Points read: {}".format(len(points)))


def reset_bit_buffer():
    global buf_byte, bit_count
    buf_byte = 0
    bit_count = 0


def byte_acc(bit):
    """
    Accumulate bits and return one byte when full

    :param bit: boolean value of next bit
    :return: a tuple in form of (result, byte). If result is True, the byte is fully formed and may be used.
    """
    global buf_byte, bit_count
    buf_byte <<= 1
    if bit:
        buf_byte |= 1
    bit_count += 1

    if bit_count == 8:
        tmp = buf_byte
        reset_bit_buffer()
        return True, tmp

    return False, None


def m_compare_per_frame():
    """
    :return: 1 byte every 16 frames
    """
    global points
    result = []
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

        full_byte, byte = byte_acc(v1 < v2)
        if full_byte:
            result.append(byte)

    return result


def m_compare_first_try():
    """
    The very first attempt
    :return:  1 byte every 8 frames
    """
    global points
    result = []

    for x, y in points:
        v1 = (x - 1) / (width - 2)
        v2 = (y - 1) / (height - 2)

        full_byte, byte = byte_acc(v1 < v2)
        if full_byte:
            result.append(byte)

    return result


def m_sha():
    """
    When we detect a flash, we feed the whole frame state to sha256 function. Thus, we guarantee that at least
    one pixel at truly random position is lit, which in turn means that the whole hash is unpredictable.
    Thermal noise of other dark pixels adds entropy. Thermal noise emulation was removed from this code because
    it makes no difference but slows things down a lot.

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

        frame = np.zeros(width * height * 2, np.dtype('b'))

        # Flash
        frame[(y * width + x) * 2] = np.random.randint(120, 127)

        h = hashlib.sha256(bytes(frame))
        np.put(result, range(offset, offset + hash_len), list(bytearray(h.digest())))
        offset += hash_len
    return result


def m_deviation():
    """
    Our coordinates are distributed somewhat normally, check deviation from mean and if it's positive,
    spawn bit 1, otherwise spawn 0 (or the other way around, it doesn't matter)
    """
    result = []
    cumulative_ma_x = 0
    cumulative_ma_y = 0
    average_window = 800

    for x, y in points:
        # Update moving average
        cumulative_ma_x = cumulative_ma_x + x - cumulative_ma_x / average_window
        cumulative_ma_y = cumulative_ma_y + y - cumulative_ma_y / average_window

        moving_average_x = cumulative_ma_x / average_window
        good_range_x = min(width - moving_average_x, moving_average_x - 0)

        moving_average_y = cumulative_ma_y / average_window
        good_range_y = min(height - moving_average_y, moving_average_y - 0)

        # Check current value deviation against the average.
        # If one side of the truncated bell is larger then the other, ignore the excess
        if x != moving_average_x and moving_average_x + good_range_x > x > moving_average_x - good_range_x:
            full_byte, byte = byte_acc(x < moving_average_x)
            if full_byte:
                result.append(byte)

        if y != moving_average_y and moving_average_y + good_range_y > y > moving_average_y - good_range_y:
            full_byte, byte = byte_acc(y < moving_average_y)
            if full_byte:
                result.append(byte)

    return result


def m_parity():
    """
    Assuming the chances of each coordinate to be even/odd are equal,
    derive one bit from coordinates parity.
    Let xp, yp be the parity of X and Y coordinates of a flash:

    | xp| yp| out
    | 0 | 0 | skip
    | 0 | 1 | bit: 1
    | 1 | 0 | bit: 0
    | 1 | 1 | skip
    """
    result = []

    for x, y in points:
        x_parity = x % 2
        y_parity = y % 2

        # De-skew attempt
        if x_parity == y_parity:
            continue

        full_byte, byte = byte_acc(x_parity < y_parity)
        if full_byte:
            result.append(byte)

    return result


def m_parity_per_frame():
    """
    Assuming the chances of each coordinate to be even/odd are equal,
    derive one bit from each coordinate parity.
    Let x1p be the parity X coordinate of a flash in previous frame and x2p the same for current frame:

    | x1p| x2p| out
    | 0 | 0 | skip
    | 0 | 1 | bit: 1
    | 1 | 0 | bit: 0
    | 1 | 1 | skip
    """
    result = []
    pv_x = -1
    pv_y = -1

    for x, y in points:
        x_parity = x % 2
        y_parity = y % 2
        if pv_x == -1:
            pv_x = x_parity
            pv_y = y_parity
            continue

        # De-skew attempt
        if pv_x != x_parity:
            full_byte, byte = byte_acc(pv_x < x_parity)
            if full_byte:
                result.append(byte)

        if pv_y != y_parity:
            full_byte, byte = byte_acc(pv_y < y_parity)
            if full_byte:
                result.append(byte)

        pv_x = -1
        pv_y = -1

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
    for x, y in points:
        x_chance = xqf_sim[x]
        y_chance = yqf_sim[y]

        full_byte, byte = byte_acc(x_chance < y_chance)
        if full_byte:
            result.append(byte)

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
        m_parity,
        m_parity_per_frame,
        m_deviation,
        m_sha,
    ]
    for v in methods:
        print("Calculating", v.__name__)
        reset_bit_buffer()  # Get rid of leftovers from previous method
        save_result(v.__name__, v)
