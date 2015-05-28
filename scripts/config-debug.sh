#!/bin/bash

[ -e configure ] || ./setup.sh
CFLAGS="-I/usr/local/include -g3 -O0" LDFLAGS=-L/usr/local/lib ./configure

# vim:ts=2:sw=2:sts=2:et:ft=sh

