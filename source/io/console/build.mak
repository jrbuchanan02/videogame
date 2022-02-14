include ./source/io/console/internal/build.mak
io_console_directories += ./source/io/console/manip ./source/io/console/internal

source_files += $(foreach dir, $(io_console_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_console_directories)

build_io_console: build_io_console_internal
	@echo Touched Console