NULL =

themedir = $(datadir)/icons/ukui-icon-theme
size = scalable
context = status

iconsdir = $(themedir)/$(size)/$(context)

icons_DATA = 			        	\
	audio-input-microphone-high.svg		\
	audio-input-microphone-low.svg		\
	audio-input-microphone-medium.svg	\
	audio-input-microphone-muted.svg	\
	$(NULL)

EXTRA_DIST =		\
	$(icons_DATA)	\
	$(NULL)


-include $(top_srcdir)/git.mk
clean-local:
	rm -f *~ Makefile.in Makefile
