#!/usr/bin/env perl

use v5.10;

use strict;
use warnings;
use autodie;

use JSON;
use Path::Class;

use constant SRC    => file "/Volumes/White Two/Timewarp/test.mp4";
use constant DST    => dir "mma";
use constant CONFIG => file "/tmp/min-max-average.json";

DST->mkpath;

for my $min ( -2 .. 2 ) {
  for my $max ( -2 .. 2 ) {
    my $avg = 1 - ( $min + $max );
    my $config = [
      { filter  => "minmax",
        options => {
          average => $avg,
          frames  => 30,
          max     => $max,
          min     => $min
        } }
    ];

    print { CONFIG->openw } JSON->new->pretty->canonical->encode($config);

    my $name = file DST, join ".", "minmax", "min=$min", "max=$max",
     "avg=$avg", "mov";

    my $cmd = join " | ",
     "ffmpeg -i '" . SRC . "' -pix_fmt yuv420p -f yuv4mpegpipe -",
     "./timewarp '" . CONFIG . "'",
     "ffmpeg -f yuv4mpegpipe -i - -c:v prores -y '$name'";
    say $cmd;
    system $cmd;
  }
}
