CFLAGS = -march=native -O1 -pipe
CC = gcc
FLAGS = -Wall -g `pkg-config --cflags --libs xcb glib-2.0`
PROGNAME = fittstool
FILES = fittstool.c

ifndef DESTDIR
	ifndef PREFIX
		BINDIR=/usr/bin
	else
		BINDIR=$(PREFIX)/bin
	endif
else
	BINDIR=$(DESTDIR)/usr/bin
endif


$(PROGNAME): 
	$(CC) $(CFLAGS) -o $(PROGNAME) $(FILES) $(FLAGS)

install:
	mkdir -p $(BINDIR)
	strip $(PROGNAME)
	install $(PROGNAME) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(PROGNAME)

clean:
	rm -rf $(PROGNAME)
