
source_ux_directories = ./source/ux/serialization

source_files += $(foreach dir, $(source_ux_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(source_ux_directories)

build_ux:
	@echo Touched UX