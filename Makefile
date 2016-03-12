CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter -pthread -lrt
OPTIMIZE = -g# -O2

PROGRAMS = addtest sltest

all: $(PROGRAMS)

addtest: addtest.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

sltest: sltest.c SortedList.c SortedList.h
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

graph:
	./run-addtest.sh
	./run-sltest.sh

DISTDIR = lab4-michaelli
DIST_FILES = Makefile *.c *.h answers.txt *.sh final-graphs

dist: $(DISTDIR)

$(DISTDIR): $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf $(PROGRAMS) graphs/ data/ $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all dist clean
