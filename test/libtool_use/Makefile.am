include  $(top_srcdir)/config/compileflags.mk
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=maintest
maintest_SOURCES=maintest.c
maintest_LDADD=libcompress.la
maintest_LDFLAGS=

#noinst_LTLIBRARIES=libcompress.la
lib_LTLIBRARIES=libcompress.la
libcompress_la_SOURCES=compress.c
libcompress_la_LIBADD=-lz
libcompress_la_LDFLAGS =

all:
	install -D ${lib_LTLIBRARIES} ${top_srcdir}/TianShan/bin64/
	cp -R .libs/libcompress.so* ${top_srcdir}/TianShan/bin64/.libs/
	install -D ${bin_PROGRAMS} ${top_srcdir}/TianShan/bin64/
clean-local:
	-rm -rf  ${lib_LTLIBRARIES} ${bin_PROGRAMS}  *.{lo,o} Makefile.in
	-rm -rf  @_bindir@/${bin_PROGRAMS} @_bindir@/${lib_LTLIBRARIES}  @_bindir@/.libs/libcompress.so*
