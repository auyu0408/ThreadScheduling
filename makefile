
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall
LIB += -ljson-c

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:simulator.o os2021_thread_api.o self_thread.o parsed_json.o function_libary.o
	$(CC) $(CFLAGS) -o simulator simulator.o os2021_thread_api.o self_thread.o parsed_json.o function_libary.o $(LIB)

simulator.o:simulator.c os2021_thread_api.h
	$(CC) $(CFLAGS) -c simulator.c $(LIB)

os2021_thread_api.o:os2021_thread_api.c os2021_thread_api.h self_thread.h function_libary.h all_include.h
	$(CC) $(CFLAGS) -c os2021_thread_api.c $(LIB)

self_thread.o:self_thread.c self_thread.h all_include.h
	$(CC) $(CFLAGS) -c self_thread.c $(LIB)

parsed_json.o:parsed_json.c parsed_json.h all_include.h
	$(CC) $(CFLAGS) -c parsed_json.c $(LIB)

function_libary.o: function_libary.c function_libary.h all_include.h
	$(CC) $(CFLAGS) -c function_libary.c $(LIB)

.PHONY: clean
clean:
	rm *.o
