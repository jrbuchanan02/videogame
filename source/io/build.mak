
io_directories = ./source/io/base

source_files += $(foreach dir, $(io_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_directories)

build_io:
	@echo Touched IO