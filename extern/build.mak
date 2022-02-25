
directories = ./extern/rapidxml-1.13 ./extern/yaml-cpp/include ./extern/yaml-cpp/src

include_dirs += $(directories)
ex_src_files += $(foreach dir, ./extern/yaml-cpp/src, $(wildcard $(dir)/*.cpp))

build_extern:
	@echo Finished Touching external dependencies