-include config.mk

SHAIFHEPATH ?= $(HOME)/HElib/src
NTLLIBPATH ?= $(HOME)/ntl-6.0.0-bin/lib
NTLINCPATH ?= $(HOME)/ntl-6.0.0-bin/include
GF2XLIBPATH ?= $(HOME)/gf2x-1.1-bin/lib

DEBUG ?= 0
ifeq ($(strip $(DEBUG)),1)
	CXXOPT :=
else
	CXXOPT := -O3 -DNDEBUG
endif

OBJDIR	 := obj
TOP	 := $(shell echo $${PWD-`pwd`})
CXX	 ?= g++
CXXFLAGS := -g $(CXXOPT) -fno-strict-aliasing -fwrapv -fPIC \
	    -Wall -Wpointer-arith -Wendif-labels -Wformat=2  \
	    -Wextra -Wwrite-strings -Wno-unused-parameter \
	    -Wno-ignored-qualifiers \
	    -Wmissing-declarations -Woverloaded-virtual  \
	    -Wunreachable-code -D_GNU_SOURCE -std=c++0x -I$(TOP) \
	    -fno-omit-frame-pointer -I$(SHAIFHEPATH) -I$(NTLINCPATH)
LDFLAGS	 := -lpthread -lrt -lgmp -lgmpxx
LDFLAGS +=  -L$(TOP)/$(OBJDIR) -Wl,-rpath=$(TOP)/$(OBJDIR) -Wl,-rpath=$(TOP)

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
.PRECIOUS: %.o %.pb.h %.pb.cc

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -MD $(CXXFLAGS) -c $< -o $@

include util/Makefrag
include math/Makefrag
include crypto/Makefrag
include mpc/Makefrag
include fhe/Makefrag
include shaifhe/Makefrag
include tree/Makefrag
include net/Makefrag
include protobuf/Makefrag

$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	perl mergedep.pl $@ $^
-include $(OBJDIR)/.deps

# .PHONY: indent
# indent:
#	uncrustify --no-backup -c conf/uncrustify.cfg $(wildcard *.cc)

# vim: set noexpandtab:
