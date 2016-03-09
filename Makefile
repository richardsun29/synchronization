CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter -pthread -lrt
OPTIMIZE = -g# -O2

PROGRAMS = addtest sltest

all: $(PROGRAMS)

addtest: addtest.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

sltest: sltest.c SortedList.c SortedList.h
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

DISTDIR = lab1-michaelli
DIST_FILES = Makefile README.md $(SRC_DIR)

dist: $(DISTDIR)

$(DISTDIR): clean $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf $(PROGRAMS) graphs/ data/ $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all dist clean
