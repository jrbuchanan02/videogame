# This file handles a recursive-building strategy to get all of the source
# files

source_files = $(wildcard ./source/*.c++)
include_dirs = ./source

windows_exec_name = videogame.exe
linux_exec_name = videogame

include ./source/build.mak

ifeq ($(operating_system), windows)
	exec_name = $(windows_exec_name)
else
	exec_name = $(linux_exec_name)
endif

build: build_source
	$(CXX) $(CXXFLAGS) $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(source_files) -o $(exec_name)

clean:
	$(RM) $(windows_exec_name) $(linux_exec_name)