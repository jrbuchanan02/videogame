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
	RM = C:\msys64\usr\bin\rm.exe -f
else
	defines = -DLINUX
	CXX = g++-10 -pthread -Wl,--no-as-needed
endif

ifeq ($(findstring --check,$(MAKEFLAGS)), --check)
	CXXFLAGS += -DUNITTEST
endif

CXXFLAGS += $(generals) $(warnings) $(optimize) $(defines)

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