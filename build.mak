# This file handles a recursive-building strategy to get all of the source
# files

source_files = $(wildcard ./source/*.c++)
include_dirs = ./source

include ./source/build.mak


build: build_source
	$(CXX) $(CXXFLAGS) $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(source_files) -o $(exec_name)
check: build_source
	$(CXX) $(CXXFLAGS) -DUNITTEST $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(source_files) -o $(exec_name)

clean:
	$(RM) $(windows_exec_name) $(linux_exec_name)