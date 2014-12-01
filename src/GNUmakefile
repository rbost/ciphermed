-include config.mk

SHAIFHEPATH ?= $(HOME)/HElib/src
NTLLIBPATH ?= $(HOME)/ntl-6.0.0-bin/lib
NTLINCPATH ?= $(HOME)/ntl-6.0.0-bin/include
JSONCPP ?= /usr/include/jsoncpp

L_BOOST_SYSTEM ?= -lboost_system
L_BOOST_THREAD ?= -lboost_thread

DEBUG ?= 0
BENCHMARK ?= 1

ifeq ($(strip $(DEBUG)),1)
	CXXOPT := -g
else
	CXXOPT := -O3 -march=native
endif

ifeq ($(strip $(BENCHMARK)),1)
	CXXOPT := $(CXXOPT) -D BENCHMARK
endif

OBJDIR	 := obj
TOP	 := $(shell echo $${PWD-`pwd`})
SHARED_OBJDIR = $(TOP)/$(OBJDIR)


CXX	 := clang++
CXXWARNINGS := -Wall -Wpointer-arith -Wendif-labels -Wformat=2 -Wextra -Wwrite-strings -Wno-unused-parameter -Wno-ignored-qualifiers -Wmissing-declarations -Woverloaded-virtual -Wunreachable-code

CXXFLAGS := $(CXXOPT) -fno-strict-aliasing -fwrapv -fPIC \
        -maes -msse4 \
        $(CXXWARNINGS) -D_GNU_SOURCE -std=c++0x  \
	    -fno-omit-frame-pointer -I$(TOP) -I$(SHAIFHEPATH) -I$(NTLINCPATH) -I$(JSONCPP) -I/usr/local/include


LDFLAGS	 := -lpthread -lgmp -lgmpxx
LDFLAGS +=  -L$(TOP)/$(OBJDIR) -rpath $(TOP)/$(OBJDIR)

PROTOC ?= protoc

## To be populated by Makefrag files
OBJDIRS	:=

.PHONY: all
all:

.PHONY: install
install:

.PHONY: clean
clean:
	rm -rf $(OBJDIR) */*.pb.h */*.pb.cc

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o %.pb.h %.pb.cc %.pb.o

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -MD $(CXXFLAGS) -c $< -o $@

include math/Makefrag
include justGarble/Makefrag
include util/Makefrag
include crypto/Makefrag
include mpc/Makefrag
include tree/Makefrag
include protobuf/Makefrag
include net/Makefrag
include classifiers/Makefrag

$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d)) $(PROTO_H)
	@mkdir -p $(@D)
	perl mergedep.pl $@ $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))

NODEPS := clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(OBJDIR)/.deps
endif

# .PHONY: indent
# indent:
#	uncrustify --no-backup -c conf/uncrustify.cfg $(wildcard *.cc)

# vim: set noexpandtab:
