DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libtimewarp.la

bin_PROGRAMS = timewarp

libtimewarp_la_SOURCES = \
	yuv4mpeg2.h yuv4mpeg2.c \
	filter.h filter.c \
	util.h util.c \
	average.h average.c \
	merge.h merge.c \
	minmax.h minmax.c \
	streak.h streak.c \
	stretch.h stretch.c \
	massive.h massive.c \
	wobble.h wobble.c \
	pass.h pass.c

libtimewarp_la_LDFLAGS = -avoid-version -static

timewarp_SOURCES = timewarp.c

timewarp_LDADD = libtimewarp.la

test: all
	cd t && $(MAKE) test

