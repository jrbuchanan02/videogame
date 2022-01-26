# This file handles a recursive-building strategy to get all of the source
# files

source_files = $(wildcard ./source/*.c++)
include_dirs = ./source

include ./source/build.mak


build: build_source
	@echo Touching Source
