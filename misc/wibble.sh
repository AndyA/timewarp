#!/bin/bash

rate=24
v4l2-ctl --set-fmt-video=width=1280,height=720,pixelformat=1
v4l2-ctl --set-parm=$rate
ffmpeg -y \
  -an -f h264 -i <( v4l2h264 -o ) \
  -pix_fmt yuv420p \
  -t 60 \
  -f yuv4mpegpipe - | ./timewarp config/wibble.json | \
  ffmpeg -y -f yuv4mpegpipe -i - -an \
    -c:v libx264 -b:v 3000k -preset veryfast c920.ts \

# vim:ts=2:sw=2:sts=2:et:ft=sh
