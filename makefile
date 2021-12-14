
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall
CLIB += -ljson-c
OBJ := simulator.o os2021_thread_api.o self_thread.o parsed_json.o function_libary.o

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:$(OBJ)
	$(CC) $(CFLAGS) -o simulator $(OBJ) $(CLIB)

simulator.o: os2021_thread_api.h
os2021_thread_api.o: os2021_thread_api.h self_thread.h function_libary.h all_include.h
self_thread.o: self_thread.h all_include.h
parsed_json.o: parsed_json.h os2021_thread_api.h all_include.h
function_libary.o: function_libary.h os2021_thread_api.h all_include.h

.PHONY: clean
clean:
	-rm $(OBJ) simulator