# Copyright (C) 2009  Mikael Larsmark, SM3WMV
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARoot directoriesRTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ========================================================================
#
# Hey Emacs, this is a -*- makefile -*-
#
# WinAVR Sample makefile written by Eric B. Weddington, J�rg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
# Additional material for this makefile was submitted by:
#  Tim Henigan
#  Peter Fleury
#  Reiner Patommel
#  Sander Pool
#  Frederik Rouleau
#  Markus Pfaff
#  Mikael Larsmark, SM2WMV
#  Tobias Blomberg, SM0SVX
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF (for use with AVR Studio 3.x or VMLAB).
#
# make extcoff = Convert ELF to AVR Extended COFF (for use with AVR Studio
#                4.07 or greater).
#
# make program = Download the hex file to the device, using avrdude.  Please
#                customize the avrdude settings below first!
#
# make filename.s = Just compile filename.c into the assembler code only
#
# To rebuild project do "make clean" then "make all".
#

# Path to project root
ROOT = ./

# MCU name
MCU = atmega128

# Target file name (without extension).
TARGET = main
# Global sources
GLOBAL_SRC =  src/delay.c src/main.c src/event_queue.c src/queue.c src/init.c src/usart.c \
							src/xbee_interface.c src/a2d.c src/helper.c

EVENT_LIST_SIZE_VAL = 10

# Board related sources
BOARD_SRC =

# List C source files here. (C dependencies are automatically generated.)
SRC = $(BOARD_SRC) $(GLOBAL_SRC)

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = src/$(ROOT) $(ROOT)/include

# Place -D or -U options here
CDEFS =

# Fuse settings. Apply by running "make fuse"
AVRDUDE_EFUSE = 0xff
AVRDUDE_HFUSE = 0xd1
AVRDUDE_LFUSE = 0x3f

# Additional paths where to look for C sources
vpath %.c $(ROOT)/misc


# Include main makefile
include $(ROOT)/makefile.inc
