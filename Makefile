CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter -pthread -lrt
OPTIMIZE = -g# -O2

PROGRAMS = addtest

all: $(PROGRAMS)

$(PROGRAMS):
	$(CC) $(CFLAGS) $(OPTIMIZE) $@.c -o $@


DISTDIR = lab1-michaelli
DIST_FILES = Makefile README.md $(SRC_DIR)

dist: $(DISTDIR)

$(DISTDIR): clean $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf addtest obj/ *.o *.tmp *.dat *.png $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean test
