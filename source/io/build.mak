
io_directories = ./source/io/base ./source/io/console ./source/io/unicode

source_files += $(foreach dir, $(io_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_directories)

include ./source/io/console/build.mak
# include ./source/io/unicode/build.mak

build_io: build_io_console
	@echo Touched IO