DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libtimewarp.la

bin_PROGRAMS = timewarp

libtimewarp_la_SOURCES = \
	yuv4mpeg2.h yuv4mpeg2.c \
	colour.h colour.c \
	model.h model.c \
	filter.h filter.c \
	average.h average.c \
	merge.h merge.c \
	peak.h peak.c \
	minmax.h minmax.c \
	streak.h streak.c \
	stretch.h stretch.c \
	massive.h massive.c \
	pass.h pass.c

libtimewarp_la_LDFLAGS = -avoid-version -static

timewarp_SOURCES = timewarp.c

timewarp_LDADD = libtimewarp.la

test: all
	cd t && $(MAKE) test

