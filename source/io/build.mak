include ./source/io/console/build.mak

io_directories = ./source/io/base ./source/io/console

source_files += $(foreach dir, $(io_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_directories)

build_io: build_io_console
	@echo Touched IO