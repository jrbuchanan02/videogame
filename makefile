cc := g++

standard := --std=gnu++2a
version := -D WINDOWS

source_directory := ./source
game_directory := ./source/game
io_directories := ./source/game/io ./source/game/io/console ./source/game/io/console/internal
engine_directory := ./source/game/engine
util_directories := ./source/util ./source/util/memory

directories := $(source_directory) $(game_directory) $(io_directories) $(engine_directory) $(util_directories)

source_files := $(foreach directory, $(directories), $(wildcard $(directory)/*.c++))
include_locs := $(foreach directory, $(directories), -I $(directory))

all:
	$(cc) $(source_files) $(include_locs) $(standard) $(version) -o ./videogame.exe