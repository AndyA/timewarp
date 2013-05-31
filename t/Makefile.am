TESTBIN = basic yuv4mpeg2 model massive merge colour hist

TESTPERL = perl.t

noinst_PROGRAMS = wrap plotpipe $(TESTBIN)

noinst_LTLIBRARIES = libtest-support.la

libtest_support_la_SOURCES = \
	tap.c tap.h framework.c framework.h \
	filter_harness.c filter_harness.h

libtest_support_la_LDFLAGS = -static

LDADD = libtest-support.la ../libtimewarp.la

test: $(TESTBIN)
	prove -e ./wrap $(TESTBIN) $(TESTPERL)

valgrind: $(TESTBIN)
	prove -e 'valgrind -q' $(addprefix ./,$(TESTBIN))
