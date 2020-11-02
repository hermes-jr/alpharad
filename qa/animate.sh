#!/bin/bash
ffmpeg -r 60 -f concat -safe 0 -i <(find . -maxdepth 1 -name 'vis_bw*.png' | sort -V | while read -r fname; do
  echo "file '$(pwd)/${fname}'"
  echo duration 0.03333
done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-bw.mp4

ffmpeg -r 60 -f concat -safe 0 -i <(find . -maxdepth 1 -name 'vis_gray*.png' | sort -V | while read -r fname; do
  echo "file '$(pwd)/${fname}'"
  echo duration 0.03333
done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-gray.mp4

ffmpeg -r 60 -f concat -safe 0 -i <(find . -maxdepth 1 -name 'plot*.png' | sort -V | while read -r fname; do
  echo "file '$(pwd)/${fname}'"
  echo duration 0.03333
done) -c:v libx264 -r 30 -pix_fmt yuv420p combined-plot.mp4
