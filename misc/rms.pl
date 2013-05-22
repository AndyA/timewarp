#!/usr/bin/env perl

use strict;
use warnings;

my @seq = (
  [0.01, 1,  100],
  [0.01, 10, 100],
  [0.01, 1,  100],
  [0.01, 10, 100],
  [0.1,  1,  100],
  [0.1,  10, 100],
  [0.1,  1,  100],
  [0.1,  10, 100],
  [1,    1,  100],
  [1,    10, 100],
  [1,    1,  100],
  [1,    10, 100],
);

my $sf  = sin_factory();
my $seq = sequencer( $sf, @seq );
my $src = rms( $seq, 0.9 );
my $p   = 0;
while ( my @data = $src->() ) {
  print join( ' ', $p++, @data ), "\n";
}

sub sequencer {
  my ( $factory, @seq ) = @_;
  my $src = undef;
  my $len = 0;
  return sub {
    until ($len) {
      return unless @seq;
      my ( $freq, $amp, $l ) = @{ shift @seq };
      $src = $factory->( $freq, $amp );
      $len = $l;
    }
    $len--;
    return $src->();
  };
}

sub rms {
  my ( $src, @decay ) = @_;

  my $avg = decay_box(@decay);
  my $d2  = decay_box(@decay);

  return sub {
    my $sample = $src->();
    return unless defined $sample;
    my $a  = $avg->($sample);
    my $dy = $sample - $a;
    return ( $sample, sqrt $d2->( $dy * $dy ) );
  };
}

sub dx_box {
  my ( $decay, $stages ) = @_;
  my @dl = map { decay_box($decay) } 1 .. $stages;
  return sub {
    my $sample = shift;
    $sample = $_->($sample) for @dl;
    return $sample;
  };
}

sub decay_box {
  my ( $decay, $ow, $nw ) = @_;
  $ow = 1 unless defined $ow;
  $nw = 1 unless defined $nw;
  my $scale = 1;
  my $acc   = 0;
  my $tw    = $ow + $nw;
  return sub {
    my $sample = shift;
    $acc = ( $acc * $decay * $ow ) + $sample * $nw;
    my $res = $acc / ( $scale * $tw );
    $scale = ( $scale * $decay * $ow ) + $nw;
    return $res;
  };
}

sub sin_factory {
  my $angle = 0;
  return sub {
    my ( $freq, $amp ) = @_;
    sub { sin( $angle += $freq ) * $amp };
  };
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

