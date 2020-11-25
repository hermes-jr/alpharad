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

# Read CSV files with analyzed observation data and render bar plots
# for method comparison

import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.ticker import ScalarFormatter, MaxNLocator

pd.set_option("display.precision", 15)

# https://tsitsul.in/blog/coloropt/
# colors = ['#ebac23', '#b80058', '#008cf9', '#006e00', '#00bbad', '#d163e6', '#b24502', '#ff9287',
#                          '#5954d6', '#00c6f8', '#878500', '#00a76c']
# colors = ['#ff9287', '#b24502', '#ebac23', '#878500', '#006e00', '#00a76c', '#00bbad', '#00c6f8', '#008cf9',
#           '#5555d5', '#d163e6', '#b80058']
colors = ['#ff9287', '#b24502', '#ebac23', '#00c6f8', '#008cf9', '#5555d5', '#878500', '#006e00', '#00a76c',
          '#00bbad', '#d163e6', '#b80058']


def grouped_horizontal_bar_plot(df, title, x_axis_major_locator, file_name):
    ax = df.plot.barh(figsize=(10, 12), width=0.65, color=colors, zorder=10)

    plt.title(title)
    plt.xscale('log')

    formatter = ScalarFormatter()
    formatter.set_scientific(False)
    ax.xaxis.set_major_formatter(formatter)
    ax.xaxis.set_major_locator(x_axis_major_locator)
    plt.grid(b=True, which='major', axis='x', zorder=0.0, alpha=0.3)

    # for p in ax.patches:
    #     ax.annotate(str(p.get_width()), (p.get_x() + p.get_width(), p.get_y()), xytext=(0.05, 1.30),
    #                 textcoords='offset points', horizontalalignment='left')

    # ax.legend(bbox_to_anchor=(0.5, 1.09), ncol=3, loc='upper center')
    handles, labels = ax.get_legend_handles_labels()
    ax.legend(handles[::-1], labels[::-1], bbox_to_anchor=(1.04, 1), loc='upper left', borderaxespad=0)

    plt.tight_layout()
    plt.savefig(file_name)


def summary_bar_plot(df_s, title, file_name):
    plt.figure()

    plt.title(title)
    plt.xscale('linear')

    ax1 = df_s['sum_e'].plot.bar(figsize=(12, 10), width=0.25, color=colors[4], position=1, rot=45)

    ax2 = ax1.twinx()
    df_s['sum_u'].plot.bar(width=0.25, color=colors[3], ax=ax2, position=0)

    handles1, _ = ax1.get_legend_handles_labels()
    handles2, _ = ax2.get_legend_handles_labels()

    labels = ['Cumulative entropy', 'Cumulative uniformity']
    ax2.legend(handles1 + handles2, labels, loc='upper left')

    ax1.set_ylabel(labels[0])
    ax2.set_ylabel(labels[1])

    plt.tight_layout()
    plt.savefig(file_name)


if __name__ == "__main__":
    df_entropy = pd.read_csv('data/analysis/2020-11-25/entropy.csv', index_col=0).T
    df_uniformity = pd.read_csv('data/analysis/2020-11-25/uniformity.csv', index_col=0).T

    grouped_horizontal_bar_plot(df_entropy, 'Entropy', MaxNLocator(nbins=10, prune=None),
                                'methods_entropy_compare.png')

    grouped_horizontal_bar_plot(df_uniformity, 'Uniformity index', MaxNLocator(nbins=4, prune='both'),
                                'methods_uniformity_compare.png')

    # Get cumulative values for summary
    df_summary = pd.DataFrame(index=df_entropy.index)
    df_summary["sum_e"] = df_entropy[df_entropy.columns[1:]].sum(axis=1)
    df_summary["sum_u"] = df_uniformity[df_uniformity.columns[1:]].sum(axis=1)

    summary_bar_plot(df_summary, 'Methods comparison summary', 'methods_summary_compare.png')
