cappletname = user-accounts
AM_CPPFLAGS = 					\
	@PACKAGE_CFLAGS@			\
	-DUIDIR="\"$(uidir)\""			\
	-fPIC					\
	-I$(srcdir)/../../shell/                \
	-DCONF_DIR=\""$(confdir)"\" 		\
        $(PANEL_CFLAGS)				\
        $(OOBS_CFLAGS)                        	\
        $(MATE_DESKTOP_CFLAGS)                  \
        $(NULL)

ccpanelsdir = $(PANELS_DIR)
ccpanels_LTLIBRARIES = libuser-accounts.la

libuser_accounts_la_SOURCES =                   \
	user-accounts.c				\
	user-accounts.h				\
	run-passwd.c				\
	run-passwd.h				\
    	check-passwd.h              		\
    	check-passwd.c              		\
	$(NULL)

libuser_accounts_la_LIBADD = 				\
			@PACKAGE_LIBS@			\
                        $(OOBS_LIBS)            	\
                        $(MATE_DESKTOP_LIBS)            \
			-lcrypt				\
			$(NULL)

libuser_accounts_la_LDFLAGS = -export-dynamic @PACKAGE_LDFLAGS@

uidir = $(pkgdatadir)/ui
ui_DATA = user-create.ui	\
	  change-pwd.ui		\
	  change-name.ui	\
	  change-face.ui	\
	  delete-user.ui	\
	  change-type.ui

EXTRA_DIST =                                    \
        $(ui_DATA)

clean-local :
	rm -f *~ Makefile.in Makefile
