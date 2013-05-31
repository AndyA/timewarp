#!/usr/bin/env perl

use strict;
use warnings;

my @h = (0) x 256;
$h[$_]++ for unpack 'C*', do { local $/; <> };

print join( ', ', @h ), "\n";

# vim:ts=2:sw=2:sts=2:et:ft=perl

