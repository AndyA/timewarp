#!/bin/bash

IN="$1"
LIMIT="$2"
[ "$LIMIT" ] && LIMIT="-t $LIMIT"

pipe="tee"

for conf in config/*.json; do
  name="$( basename "$conf" .json )"
  out=$( echo "$IN" | perl -pe "s@([^/.]+)\.[^/.]+\$@$name-\$1.ts@"  )
  cmd="./timewarp $conf | ffmpeg -y -f yuv4mpegpipe -i - -map 0:0 -c:v libx264 -b:v 3000k $out"
  pipe="$pipe >( $cmd )"
done
pipe="$pipe > /dev/null"
echo "$pipe"

set -x
ffmpeg -y -i "$IN" $LIMIT -f yuv4mpegpipe - | eval $pipe


# vim:ts=2:sw=2:sts=2:et:ft=sh

