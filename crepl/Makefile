NAME := $(shell basename $(PWD))
export MODULE := M4
all: $(NAME)-64 $(NAME)-32
LDFLAGS += -ldl
32:all
	./crepl-32
64:all
	./crepl-64

include ../Makefile
