.PHONY: all
all:
	gcc chatserver.c -o chatserver -lpthread
	gcc chatclient.c -o chatclient -lpthread