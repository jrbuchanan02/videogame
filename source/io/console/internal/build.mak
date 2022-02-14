io_console_internal_directories += ./source/io/console/colors

source_files += $(foreach dir, $(io_console_internal_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(io_console_internal_directories)

build_io_console_internal:
	@echo Touched Console's Internals