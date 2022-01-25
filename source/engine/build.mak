
engine_dirs = ./source/engine/rand

source_files += $(foreach dir, $(engine_dirs), $(wildcard $(dir)/*.c++))
include_dirs += $(engine_dirs)


build_engine:
	@echo Touched Engine