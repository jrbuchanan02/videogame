################################################################################
# OS-specific defines
################################################################################
mingw_make := mingw32-make.exe
linux_make := make

# set from commandline
INTERNAL_CHAR ?= 1
EXTERNAL_CHAR ?= 1

ifeq ($(findstring $(mingw_make), $(notdir $(MAKE))), $(mingw_make))
	operating_system := windows
else
	operating_system := linux
endif

warnings = -Wall -Wpedantic -Werror
optimize = -Ofast
debugging = -g -Ofast
generals = --std=gnu++2a

ifeq ($(operating_system), windows)
	defines = -DWINDOWS
	clang_format = @echo "Would run clang-format-12"
	RM = C:\msys64\usr\bin\rm.exe -f
else
	defines = -DLINUX
	clang_format = clang-format-12
	CXX = g++-10 -pthread -Wl,--no-as-needed
endif

defines += -DI_CHAR_SIZE=$(INTERNAL_CHAR) -DE_CHAR_SIZE=$(EXTERNAL_CHAR)

CXXFLAGS += $(generals) $(warnings) $(defines)
debug: CXXFLAGS += $(debugging)
all: CXXFLAGS += $(optimize)

windows_exec_name = videogame.exe
linux_exec_name = videogame.out
ifeq ($(operating_system), windows)
	exec_name = $(windows_exec_name)
else
	exec_name = $(linux_exec_name)
endif


defs_test:
	@echo $(MAKE)
	@echo $(mingw_make)
	@echo $(linux_make)
	@echo $(operating_system)
	@echo $(CXXFLAGS)