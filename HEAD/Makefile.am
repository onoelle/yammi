####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = yammi po 

EXTRA_DIST = yammi.kdevprj admin CHANGELOG yammiGrabAndEncode AUTHORS INSTALL README TODO CREDITS yammiMp3Edit yammi.lsm 

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin/
	$(INSTALL_SCRIPT) yammiGrabAndEncode $(prefix)/bin/yammiGrabAndEncode

uninstall-local:
	-rm -f $(prefix)/bin/yammiGrabAndEncode

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 

#$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
