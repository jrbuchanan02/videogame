################################################################################
# OS-specific defines
################################################################################
mingw_make := mingw32-make.exe
linux_make := make

ifeq ($(findstring $(mingw_make), $(notdir $(MAKE))), $(mingw_make))
	operating_system := windows
else
	operating_system := linux
endif

warnings = -Wall -Wpedantic -Werror
optimize = -Ofast
generals = --std=gnu++2a

ifeq ($(operating_system), windows)
	defines = -DWINDOWS
	RM = @echo
else
	defines = -DLINUX
	CXX = g++-10 -pthread -Wl,--no-as-needed
endif

CXXFLAGS += $(generals) $(warnings) $(optimize) $(defines)

VPATH += ./
VPATH += ./source
VPATH += ./init

windows_exec_name = videogame.exe
linux_exec_name = videogame.out
ifeq ($(operating_system), windows)
	exec_name = $(windows_exec_name)
else
	exec_name = $(linux_exec_name)
endif

source = 


defs_test:
	@echo $(MAKE)
	@echo $(mingw_make)
	@echo $(linux_make)
	@echo $(operating_system)
	@echo $(CXXFLAGS)