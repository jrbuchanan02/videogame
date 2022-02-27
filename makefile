include defines.mak
include build.mak

.DEFAULT_TARGET: all

object_files = $(source_files:%.c++=%.o)
ex_obj_files = $(ex_src_files:%.cpp=%.o)



define compile
	@echo Compiling file $@ with flags $(CXXFLAGS)
	@$(CXX) $(CXXFLAGS) -DUNITTEST $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(patsubst %.o,%.c++,$@) -c -o $@
endef

define ex_compile
	@echo Compiling file $@ with flags $(CXXFLAGS)
	@$(CXX) $(CXXFLAGS) -DUNITTEST $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(patsubst %.o,%.cpp,$@) -c -o $@
endef


formatted_source = $(source_files) $(foreach dir, $(include_dirs), $(wildcard $(dir)/*.h++))

$(object_files) : $(source_files)
	$(compile)
$(ex_obj_files) : $(ex_src_files)
	$(ex_compile)

dummy_remove: $(source_files) $(ex_src_files)
	@echo Here!
	rm -Force $(windows_exec_name) $(linux_exec_name)
	rm -Force $(object_files)
	rm -Force $(ex_obj_files)

clean: $(source_files) $(ex_src_files)
	$(RM) $(windows_exec_name) $(linux_exec_name)
	$(RM) $(object_files)
	$(RM) $(ex_obj_files)

all: build $(object_files) $(ex_obj_files)
	@echo Linking...
	@$(CXX) $(CXXFLAGS) $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(object_files) $(ex_obj_files) -o $(exec_name)
	@echo Built the following source files and included the following directories.
	@echo $(source_files)
	@echo $(include_dirs)

debug: build $(object_files) $(ex_obj_files)
	@echo Linking...
	@$(CXX) $(CXXFLAGS) $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(object_files) $(ex_obj_files) -o $(exec_name)
	@echo Built the following source files and included the following directories.
	@echo $(source_files)
	@echo $(include_dirs)

check: all
	./$(exec_name) --unittest

do_format: $(formatted_source)
	@echo Formatting...
	$(clang_format) -style=file -i $(formatted_source)

install:
	@echo Installing required dependencies for building.
	@echo This command requires Ubuntu 20.04 LTS and also requires sudo permissions.
	@echo All sudo commands will be shown. 
	@echo Note that the command may function on similar versions of linux that have
	@echo apt-get
	apt update
	apt install gcc-10 -y
	apt install clang-format-12 -y
