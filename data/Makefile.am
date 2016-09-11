SUBDIRS = images

pkgdata_DATA = select.cl

# Install a link for compatibility (TODO: Remove!)
install-data-local :
	rm -f $(pkgdatadir)/lib
	ln -s ./ $(pkgdatadir)/lib

EXTRA_DIST = $(pkgdata_DATA)
