####### kdevelop will overwrite this part!!! (begin)##########

# TODO: I did not find how to do this with automake manager, so I edited this file by hand
# ...I start to hate automake manager
# - how is it supposed to work?
# - and where do I find good docs (and not "to be written..." all over)


EXTRA_DIST = in.xpm notin.xpm newCategory.xpm some_in.xpm yammi16x16.png yammi32x32.png yammi48x48.png yammi64x64.png stop_prelisten.png prelisten_start.png prelisten_middle.png prelisten_end.png play_now.png enqueue_asnext.png enqueue.png toggle_playlist.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/
	
	$(INSTALL_DATA) $(srcdir)/enqueue_asnext.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue_asnext.png
	$(INSTALL_DATA) $(srcdir)/enqueue.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue.png
	$(INSTALL_DATA) $(srcdir)/play_now.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/play_now.png
	$(INSTALL_DATA) $(srcdir)/stop_prelisten.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/stop_prelisten.png
	$(INSTALL_DATA) $(srcdir)/prelisten_start.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_start.png
	$(INSTALL_DATA) $(srcdir)/prelisten_middle.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_middle.png
	$(INSTALL_DATA) $(srcdir)/prelisten_end.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_end.png
	$(INSTALL_DATA) $(srcdir)/toggle_playlist.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/toggle_playlist.png

	$(mkinstalldirs) $(kde_icondir)/crystalsvg/{16x16,22x22,32x32,48x48,64x64}/apps/

	$(INSTALL_DATA) $(srcdir)/yammi16x16.png $(kde_icondir)/crystalsvg/16x16/apps/yammi.png
	$(INSTALL_DATA) $(srcdir)/yammi22x22.png $(kde_icondir)/crystalsvg/22x22/apps/yammi.png
	$(INSTALL_DATA) $(srcdir)/yammi32x32.png $(kde_icondir)/crystalsvg/32x32/apps/yammi.png
	$(INSTALL_DATA) $(srcdir)/yammi48x48.png $(kde_icondir)/crystalsvg/48x48/apps/yammi.png
	$(INSTALL_DATA) $(srcdir)/yammi64x64.png $(kde_icondir)/crystalsvg/64x64/apps/yammi.png

uninstall-local:
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue_asnext.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/play_now.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_start.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_middle.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_end.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/stop_prelisten.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/toggle_playlist.png
	-rm -f $(kde_icondir)/crystalsvg/16x16/apps/yammi.png
	-rm -f $(kde_icondir)/crystalsvg/22x22/apps/yammi.png
	-rm -f $(kde_icondir)/crystalsvg/32x32/apps/yammi.png
	-rm -f $(kde_icondir)/crystalsvg/48x48/apps/yammi.png
	-rm -f $(kde_icondir)/crystalsvg/64x64/apps/yammi.png

####### kdevelop will overwrite this part!!! (end)############
