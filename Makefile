# Compiler and flags
CC = gcc
CFLAGS_DEBUG   = -Wall -Wextra -g -O0 
CFLAGS_RELEASE = -Wall -Wextra -O3 -DNDEBUG

LDFLAGS = -lpretty

# Project structure
SRC = main.c cli.c encode.c decode.c
OBJ_DEBUG   = $(patsubst %.c, out/debug/%.o, $(SRC))
OBJ_RELEASE = $(patsubst %.c, out/release/%.o, $(SRC))

TARGET_DEBUG   = out/debug/qoi_tool
TARGET_RELEASE = out/release/qoi_tool

# Default action
all: release

# ======================
# Build rules
# ======================

debug: $(TARGET_DEBUG)

release: $(TARGET_RELEASE)

# Create output dirs
out/debug:
	mkdir -p out/debug

out/release:
	mkdir -p out/release

# Debug object files
out/debug/%.o: %.c | out/debug
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@ 

# Release object files
out/release/%.o: %.c | out/release
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

# Final linked binaries
$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(OBJ_DEBUG) -o $(TARGET_DEBUG) $(LDFLAGS)

$(TARGET_RELEASE): $(OBJ_RELEASE)
	$(CC) $(OBJ_RELEASE) -o $(TARGET_RELEASE) $(LDFLAGS)

# ======================
# Housekeeping
# ======================

clean:
	rm -rf out/debug/*.o out/release/*.o

distclean:
	rm -rf out

.PHONY: all debug release clean distclean
