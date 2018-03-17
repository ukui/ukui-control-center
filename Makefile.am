SUBDIRS = data panels shell po man

DIST_SUBDIRS = data panels shell po man

@INTLTOOL_XML_NOMERGE_RULE@
gsettingsschema_in_files = \
	org.ukui.control-center.keybinding.gschema.xml.in

gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
@GSETTINGS_RULES@

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

EXTRA_DIST = \
	autogen.sh \
	$(gsettingsschema_in_files)

clean-local :
	rm -rf config.guess config.sub config.* aclocal.m4 libtool compile configure Makefile.in install-sh missing autom4te.cache config.h.in ltmain.sh depcomp m4 config.h.in~ INSTALL Makefile stamp-h1 po/Makefile.in.in po/Makefile po/Makefile.in po/POTFILES man/Makefile.in man/Makefile
