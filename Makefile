CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter -pthread
OPTIMIZE = -g# -O2

SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: addtest

addtest: $(OBJ_DIR) $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -c $< -o $@

DISTDIR = lab1-michaelli
DIST_FILES = Makefile README.md $(SRC_DIR)

dist: $(DISTDIR)

$(DISTDIR): clean $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf addtest obj/ *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean test
