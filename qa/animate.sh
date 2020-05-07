#!/bin/bash
ffmpeg -r 30 -f concat -safe 0 -i <(ls -1v vis_bw*png | while read fname; do echo "file '$(pwd)/${fname}'"; done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-bw.mp4
ffmpeg -r 30 -f concat -safe 0 -i <(ls -1v vis_gray*png | while read fname; do echo "file '$(pwd)/${fname}'"; done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-gray.mp4
