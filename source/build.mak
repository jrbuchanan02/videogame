include ./source/io/build.mak
include ./source/engine/build.mak

source_directories = ./source/init ./source/io ./source/engine

source_files += $(foreach dir, $(source_directories), $(wildcard $(dir)/*.c++))
include_dirs += $(source_directories)

build_source: build_io build_engine
	@echo Touched Source!