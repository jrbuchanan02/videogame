
io_console_directories += ./source/io/console/manip ./source/io/console/internal ./source/io/console/colors

source_files += $(foreach dir, $(io_console_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_console_directories)

build_io_console: 
	@echo Touched Console