# Project name
PROJECT = stub

# Imported files
CHIBIOS  := $(CHIBIOS_SOURCE_PATH)

# Directories
CONFDIR  := ./config
BUILDDIR := ./build
DEPDIR   := ./build/dep
BOARDDIR := ./build/board

# Includes
ALLINC += src

# Source files
CSRC =	$(ALLCSRC)					\
		src/main.c					\
									\
		src/peripherals.c			\
		src/peripherals/ltc6811.c	\
									\
		src/can_vehicle.c			\
		src/can/receive.c			\
		src/can/transmit.c			\
									\
		src/watchdog.c

# Common library includes
include common/src/debug.mk
include common/src/fault_handler.mk

include common/src/can/can_thread.mk

include common/src/peripherals/analog_linear.mk
include common/src/peripherals/mc24lc32.mk

# Compiler flags
USE_OPT = -Og -Wall -Wextra

# C macro definitions
UDEFS =

# ASM definitions
UADEFS =

# Include directories
UINCDIR =

# Library directories
ULIBDIR =

# Libraries
ULIBS =

# ChibiOS extra includes
include $(CHIBIOS)/os/hal/lib/streams/streams.mk

# Common toolchain includes
include common/makefile

# ChibiOS compilation hooks
PRE_MAKE_ALL_RULE_HOOK: $(BOARD_FILES) $(CLANGD_FILE)