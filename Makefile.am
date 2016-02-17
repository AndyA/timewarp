DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libtimewarp.la

bin_PROGRAMS = timewarp pipecleanr y4m2patch

libtimewarp_la_SOURCES = \
	average.h average.c \
	colour.h colour.c \
	filter.h filter.c \
	hist.h hist.c \
	histogram.h histogram.c \
	massive.h massive.c \
	merge.h merge.c \
	minmax.h minmax.c \
	mma.h mma.c \
	model.h model.c \
	pass.h pass.c \
	peak.h peak.c \
	streak.h streak.c \
	stretch.h stretch.c \
	ripple.h ripple.c \
	yuv4mpeg2.h yuv4mpeg2.c

libtimewarp_la_LDFLAGS = -avoid-version -static

timewarp_SOURCES = timewarp.c

timewarp_LDADD = libtimewarp.la

pipecleanr_SOURCES = pipecleanr.c

pipecleanr_LDADD = libtimewarp.la

y4m2patch_SOURCES = y4m2patch.c

y4m2patch_LDADD = libtimewarp.la


test: all
	cd t && $(MAKE) test
