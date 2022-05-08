NAME := $(shell basename $(PWD))
export MODULE := M4
all: $(NAME)-64 $(NAME)-32
LDFLAGS += -ldl -DLOCAL_MACHINE
32:all
	./crepl-32
64:all
	./crepl-64
remove:clean
	rm func.c
	rm test
include ../Makefile
