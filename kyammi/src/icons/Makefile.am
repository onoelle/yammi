####### kdevelop will overwrite this part!!! (begin)##########

# TOD: I did not find how to do this with automake manager, so I edited this file by hand
# I start to hate automake manager
# - how is it supposed to work?
# - and where do I find good docs (and not "to be written..." all over)


EXTRA_DIST = clearPlaylist.xpm defaultControlClick.xpm defaultDoubleClick.xpm defaultMiddleClick.xpm defaultShiftClick.xpm in.xpm notin.xpm pause.xpm skipbackward.xpm skipforward.xpm stop.xpm newCategory.xpm some_in.xpm toggle_playlist.png yammi16x16.png yammi32x32.png yammi48x48.png yammi64x64.png stop_prelisten.png prelisten_start.png prelisten_middle.png prelisten_end.png play_now.png play.png enqueue_asnext.png dequeue_all.png dequeue_song.png enqueue.png song_info.png 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/
	
	$(INSTALL_DATA) $(srcdir)/enqueue_asnext.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue_asnext.png
	$(INSTALL_DATA) $(srcdir)/enqueue.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue.png
	$(INSTALL_DATA) $(srcdir)/play_now.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/play_now.png
	$(INSTALL_DATA) $(srcdir)/stop_prelisten.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/stop_prelisten.png
	$(INSTALL_DATA) $(srcdir)/prelisten_start.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_start.png
	$(INSTALL_DATA) $(srcdir)/prelisten_middle.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_middle.png
	$(INSTALL_DATA) $(srcdir)/prelisten_end.png $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_end.png
	
	
dont-call:	
	$(mkinstalldirs) $(kde_datadir)/yammi/
	$(INSTALL_DATA) $(srcdir)/toggle_playlist.png $(kde_datadir)/yammi/toggle_playlist.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/yammi16x16.png $(kde_icondir)/hicolor/16x16/apps/yammi.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/yammi32x32.png $(kde_icondir)/hicolor/32x32/apps/yammi.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/yammi48x48.png $(kde_icondir)/hicolor/48x48/apps/yammi.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/64x64/apps/
	$(INSTALL_DATA) $(srcdir)/yammi64x64.png $(kde_icondir)/hicolor/64x64/apps/yammi.png
	$(INSTALL_DATA) $(srcdir)/play_now.png $(kde_datadir)/yammi/icons/hicolor/16x16/actions/play_now.png
	$(mkinstalldirs) $(kde_datadir)/yammi/icons/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/play.png $(kde_datadir)/yammi/icons/hicolor/16x16/actions/play.png

uninstall-local:
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue_asnext.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/enqueue.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/play_now.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_start.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_middle.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/prelisten_end.png
	-rm -f $(kde_datadir)/yammi/icons/crystalsvg/22x22/actions/stop_prelisten.png

dont-call2:
	-rm -f $(kde_datadir)/yammi/toggle_playlist.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/yammi.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/yammi.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/yammi.png
	-rm -f $(kde_icondir)/hicolor/64x64/apps/yammi.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/stop_prelisten.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/prelisten_start.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/prelisten_middle.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/prelisten_end.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/play_now.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/play.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/enqueue_asnext.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/dequeue_all.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/dequeue_song.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/enqueue.png
	-rm -f $(kde_datadir)/yammi/icons/hicolor/16x16/actions/song_info.png

####### kdevelop will overwrite this part!!! (end)############
