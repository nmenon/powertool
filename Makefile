#
# Make file for compiling powertool for beaglebone black
#
# (C) Copyright 2014
# Texas Instruments, <www.ti.com>
# Nishanth Menon <nm@ti.com>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation version 2.
#
# This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
# whether express or implied; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA

# Handle verbose
ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif
Q := $(if $(VERBOSE:1=),@)

# Get project file details from Makefile.project
include Makefile.project

# on ubuntu, sudo apt-get install gcc-arm-linux-gnueabi
#CROSS_COMPILE ?= arm-linux-gnueabi-

# Program name definition for ARM GNU C compiler.
CROSS_CC = ${CROSS_COMPILE}gcc
# Program name definition for ARM GNU Linker.
CROSS_LD = ${CROSS_COMPILE}gcc
# Program name definition for ARM GNU Strip
CROSS_STRIP = ${CROSS_COMPILE}strip

# Flags for LD
CROSS_LFLAGS += --gc-sections --static -lm

#Includes for our compiler
CROSS_CFLAGS +=-Os -ffunction-sections -fdata-sections -Wall -pedantic -c -g

# I want to save the path to libgcc, libc.a and libm.a for linking.
# I can get them from the gcc frontend, using some options.
# See gcc documentation
CROSS_LIB_GCC_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-libgcc-file-name}
CROSS_LIBC_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-file-name=libc.a}
CROSS_LIBM_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-file-name=libm.a}

#==============================================================================
# Rules to make the target
#==============================================================================
CROSS_ALL_SOURCES = ${PROJECT_SRC}
CROSS_OBJS = $(CROSS_ALL_SOURCES:.c=.o)
CROSS_DEP += $(CROSS_ALL_SOURCES:.c=.d)

.PHONY:	tags cscope

#make all rule
all: $(CROSS_OBJS) ${PROJECT_NAME} ${PROJECT_NAME}

${PROJECT_NAME}: $(CROSS_OBJS)
	@echo Linking...
	$(Q)$(CROSS_LD) $(CROSS_LFLAGS)\
		-o ${PROJECT_NAME} $(CROSS_OBJS)
	$(Q)$(CROSS_STRIP) ${PROJECT_NAME}

# pull in dependency info for *existing* .o files
-include ${CROSS_DEP}

%.o: %.c
	@echo Compiling $< ...
	$(Q)$(CROSS_CC) -c $(CROSS_CFLAGS) -MD ${<} -o ${@}

# make clean rule
clean:
	$(Q)rm -f *.bin *.o *.d *.axf *.lst ${PROJECT_NAME} $(CROSS_OBJS)\
		${CROSS_DEP} tags cscope.out

# Tags stuff..
tags: $(CROSS_ALL_SOURCES)
	@echo "Building tags.."
	$(Q)ctags $(shell $(CROSS_CC) $(CROSS_CFLAGS) -MM -MG\
		$(CROSS_ALL_SOURCES) | sed -e "s/^.*\.o://g"|tr -d '\\')

# Build cscope db as well
cscope: $(CROSS_ALL_SOURCES)
	@echo "Building cscope db.."
	$(Q)cscope -b $(shell $(CROSS_CC) $(CROSS_CFLAGS) -MM -MG\
		$(CROSS_ALL_SOURCES) | sed -e "s/^.*\.o://g"|tr -d '\\')
