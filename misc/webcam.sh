#!/bin/bash

rate=15
v4l2-ctl --set-fmt-video=width=1280,height=720,pixelformat=1
v4l2-ctl --set-parm=$rate
ffmpeg -y \
  -f alsa -ac 2 -r:a 32000 -i hw:1,0 \
  -f h264 -i <( v4l2h264 -o ) \
  -acodec libfaac -b:a 128k -ac 2 \
  -vcodec copy \
  -t 60 \
  c920.ts

# vim:ts=2:sw=2:sts=2:et:ft=sh
