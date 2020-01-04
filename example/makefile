.PHONY: default
example_name=ipc
prefix=.exe
lib_dir=../build
lib_name=libipc.a
dep =$(lib_dir)/$(lib_name)
FLAGS=-L$(lib_dir) -l:$(lib_name) -std=c++11 -lpthread -g

default: ${example_name}_server${prefix} ${example_name}_client${prefix}

${example_name}_server${prefix}: server.cpp $(dep)
	g++  $< ${FLAGS} -o $@

${example_name}_client${prefix}: client.cpp $(dep)
	g++  $< ${FLAGS} -o $@

run:
	./${example_name}_server${prefix} &
	./${example_name}_client${prefix}

clean:
	rm -rf *.exe *~ *.o

.PHONY: $(TARGET)