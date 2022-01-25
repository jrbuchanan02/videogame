include defines.mak
include build.mak


all: defs_test clean build
	@echo Built the following source files and included the following directories.
	@echo $(source_files)
	@echo $(include_dirs)
