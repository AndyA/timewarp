DIST_SUBDIRS = m4 t

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = timewarp

timewarp_SOURCES = timewarp.c

test: all
	cd t && $(MAKE) test

