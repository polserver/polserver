# Makefile for ancient gcc4 (tested on Ubuntu 7.10 with 4.1.2-16ubuntu2)
# Compiles Kaitai Struct C++ runtime and tests

BUILD_DIR := build

SOURCES := \
	kaitai/kaitaistream.cpp \
	tests/unittest.cpp

OBJS := \
	$(BUILD_DIR)/kaitaistream.o \
	$(BUILD_DIR)/unittest.o

DEFINES := -DKS_STR_ENCODING_ICONV -DGTEST_NANO

LDFLAGS :=
LDLIBS :=

INCLUDES := -I.

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/unittest

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/unittest: $(OBJS)
	$(CXX) $(CXXFLAGS) -o build/unittest $(OBJS) $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/kaitaistream.o: kaitai/kaitaistream.cpp kaitai/kaitaistream.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(BUILD_DIR)/unittest.o: tests/unittest.cpp tests/gtest-nano.h kaitai/kaitaistream.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
