#!/bin/bash
ffmpeg -r 120 -f concat -safe 0 -i <(ls -1v vis_bw*png | while read fname; do echo "file '$(pwd)/${fname}'"; echo duration 0.03333; done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-bw.mp4
ffmpeg -r 120 -f concat -safe 0 -i <(ls -1v vis_gray*png | while read fname; do echo "file '$(pwd)/${fname}'"; echo duration 0.03333; done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-gray.mp4
ffmpeg -r 120 -f concat -safe 0 -i <(ls -1v plot_*png | while read fname; do echo "file '$(pwd)/${fname}'"; echo duration 0.03333; done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-plot.mp4
