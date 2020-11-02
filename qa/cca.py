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

Analyze connected components
"""

file_name = '../points.log'
diff_threshold = 10


def read_data():
    raw_data = []
    with open(file_name, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            coordinates = line.split(':')
            raw_data.append((int(coordinates[0]), int(coordinates[1])))
    return raw_data


def find_connections(raw_data):
    """
    Detect runs with absolute difference of no more than 1 in any dimension

    Also limit by diff_threshold so that flashes detected on the same line but located far apart
    would not be counted as one flash. It's a dirty workaround but as long as result is ok, it may be neglected

    :param raw_data: a list of x:y coordinates
    :return: adjacency matrix, result[i] = True if raw_data[i] and raw_data[i+1] are neighbors, False otherwise
    """
    n_gaps = len(raw_data) - 1
    conn = [False] * n_gaps
    for i in range(n_gaps):
        x1, y1 = raw_data[i]
        x2, y2 = raw_data[i + 1]
        diff_x = abs(x1 - x2)
        diff_y = abs(y1 - y2)
        if (diff_x < 2 and diff_y < diff_threshold) or (diff_y < 2 and diff_x < diff_threshold):
            conn[i] = True
    return conn


def split_connections(conn, raw_data):
    """
    Split raw_data into groups using given adjacency matrix

    :param conn: result of find_connections()
    :param raw_data: a list of x:y coordinates
    :return: a list of coordinate groups, each group consists of neighboring pixels
    """
    groups = []
    cur_group = [raw_data[0]]
    for i in range(1, len(raw_data)):
        if conn[i - 1]:
            cur_group.append(rd[i])
        else:
            groups.append(cur_group)
            cur_group = [rd[i]]
    groups.append(cur_group)
    return groups


def analyze_groups(groups):
    """
    Print each pattern along with its frequency of appearance

    :param groups: previously grouped pixels
    """
    coll = dict()

    print('Normalized:')
    for g in groups:
        min_x = min(x for x, _ in g)
        min_y = min(y for _, y in g)
        g_norm = tuple(sorted(((x - min_x, y - min_y) for x, y in g)))
        print(g_norm)
        coll[g_norm] = coll.get(g_norm, 0) + 1

    print('Patterns in order of descending frequency of appearance:')
    for coordinates, pattern_count in sorted(coll.items(), key=lambda item: item[1], reverse=True):
        print('\n==============\n')

        max_x = max(x for x, _ in coordinates)
        max_y = max(y for _, y in coordinates)
        render = [[' ' for _ in range(max_x + 1)] for _ in range(max_y + 1)]
        for x, y in coordinates:
            render[y][x] = '#'

        for i in range(max_y + 1):
            for j in range(max_x + 1):
                print(render[i][j], end='')
            if i == 0:
                print('    {} ({}%)'.format(pattern_count, round(100.0 * coll[coordinates] / len(groups), 4)))
            else:
                print('')

    print('Total groups: {}\n'.format(len(coll)))


def print_with_connections():
    for k, (x, y) in enumerate(rd):
        print('* {}:{}'.format(x, y))
        if k < len(cc):
            print('|' if cc[k] else ' ')


def cleanup_attempt(groups):
    """
    Use some strategy to get rid of clusters, yet leaving the resulting numbers acceptably distributed

    :param groups: all groups of points
    """
    cleaned_up = []
    ctr = 0
    for g in groups:
        lucky = g[ctr % len(g)]
        cleaned_up.append(lucky)
        ctr = max(ctr + 1, 0)  # signed
    return cleaned_up


if __name__ == "__main__":
    # Read file
    rd = read_data()
    print('{} points read:\n'.format(len(rd)))

    # Detect connected pixels
    cc = find_connections(rd)
    print_with_connections()

    # Split them into groups
    sc = split_connections(cc, rd)

    # Analyze groups, print stats
    analyze_groups(sc)

    clean = cleanup_attempt(sc)
    print('Cleaned up list:')
    for p in clean:
        print('{}:{}'.format(p[0], p[1]))
