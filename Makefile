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

VERSION_MAJOR=0
VERSION_MINOR=1

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
ifeq (${ARCH}, "sandbox")
  CROSS_STRIP = /bin/true
else
  CROSS_STRIP = ${CROSS_COMPILE}strip
endif

# Flags for LD
CROSS_LFLAGS += -lm
ifneq ($(ARCH),sandbox)
  CROSS_LFLAGS += --gc-sections --static
endif

# Includes for our compiler
CROSS_CFLAGS += -I ./src -I ./include -I ./lib

# Generic compiler stuff
CROSS_CFLAGS +=-Os -ffunction-sections -fdata-sections -Wall -c -g

# libcfg
CROSS_CFLAGS += -I ./lib/lcfg/
CROSS_ALL_SOURCES += ./lib/lcfg/lcfg_static.c

# lib-i2c
CROSS_CFLAGS += -I ./lib/i2c-tools/include -I ./lib/i2c-tools/tools/
CROSS_ALL_SOURCES += ./lib/i2c-tools/tools/i2cbusses.c
LIB_I2C_REV_FILE = ./lib/i2c-tools/version.h

# omapconf
CROSS_CFLAGS += -I ./lib/omapconf/common
CROSS_ALL_SOURCES += ./lib/omapconf/common/autoadjust_table.c

# Generated version file:
CROSS_ALL_SOURCES += ./src/version.c
VERSION_GEN_FILE = ./src/version_gen.c
CROSS_VER_OBJ = $(VERSION_GEN_FILE:.c=.o)

# I want to save the path to libgcc, libc.a and libm.a for linking.
# I can get them from the gcc frontend, using some options.
# See gcc documentation
CROSS_LIB_GCC_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-libgcc-file-name}
CROSS_LIBC_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-file-name=libc.a}
CROSS_LIBM_PATH=${shell ${CROSS_CC} ${CROSS_CFLAGS} -print-file-name=libm.a}

#==============================================================================
# Rules to make the target
#==============================================================================
CROSS_ALL_SOURCES += ${PROJECT_SRC}
CROSS_OBJS = $(CROSS_ALL_SOURCES:.c=.o)
CROSS_DEP += $(CROSS_ALL_SOURCES:.c=.d)
CROSS_REV_DEP += $(VERSION_GEN_FILE:.c=.d)

.PHONY:	tags cscope

#make all rule
all: ${PROJECT_NAME}

${PROJECT_NAME}: $(CROSS_OBJS) ${CROSS_VER_OBJ}
	@echo Linking...
	$(Q)$(CROSS_LD) $(CROSS_LFLAGS)\
	  -o ${PROJECT_NAME} $(CROSS_OBJS) $(CROSS_VER_OBJ)
	$(Q)$(CROSS_STRIP) ${PROJECT_NAME}
	@echo Done. Binary '"'${PROJECT_NAME}'"' is ready.

${VERSION_GEN_FILE}: $(CROSS_OBJS)
	$(Q)echo '#include <version.h>' >${VERSION_GEN_FILE} &&\
		echo 'char *powertool_version = "'`git describe --dirty\
			2>/dev/null ||\
		echo "$(VERSION_MAJOR).$(VERSION_MINOR)-nogit"`'";' >>\
		${VERSION_GEN_FILE} &&\
	   cat ${LIB_I2C_REV_FILE}>> ${VERSION_GEN_FILE} &&\
	   echo 'char *lib_i2c_revision = VERSION;'>>${VERSION_GEN_FILE} &&\
	   echo 'char *powertool_builddate = "'`date`'";' >>${VERSION_GEN_FILE}

# pull in dependency info for *existing* .o files
-include ${CROSS_DEP}

%.o: %.c
	@echo Compiling $< ...
	$(Q)$(CROSS_CC) -c $(CROSS_CFLAGS) -MD ${<} -o ${@}
# make clean rule
clean:
	$(Q)rm -f *.bin *.o *.d *.axf *.lst ${PROJECT_NAME} $(CROSS_OBJS)\
		${CROSS_DEP} tags cscope.out ${VERSION_GEN_FILE}\
		$(CROSS_VER_OBJ) $(CROSS_REV_DEP)

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
