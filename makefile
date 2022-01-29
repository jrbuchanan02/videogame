include defines.mak
include build.mak

.DEFAULT_TARGET: all

object_files = $(source_files:%.c++=%.o)

define compile
	@echo Compiling file $@ with flags $(CXXFLAGS)
	$(CXX) $(CXXFLAGS) -DUNITTEST $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(patsubst %.o,%.c++,$@) -c -o $@
endef


$(object_files) : $(source_files)
	$(compile)

dummy_remove: $(source_files)
	@echo Here!
	rm -Force $(windows_exec_name) $(linux_exec_name)
	rm -Force $(object_files)

clean: $(source_files)
	$(RM) $(windows_exec_name) $(linux_exec_name)
	$(RM) $(object_files)

all: build $(object_files)
	@echo Performing final linkage...
	$(CXX) $(CXXFLAGS) $(foreach inc_dir, $(include_dirs), -I $(inc_dir)) $(object_files) -o $(exec_name)
	@echo Built the following source files and included the following directories.
	@echo $(source_files)
	@echo $(include_dirs)

check: all