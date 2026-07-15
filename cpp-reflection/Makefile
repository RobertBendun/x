all: print-struct lisp

%: %.cc
	g++ -std=c++26 -freflection -Wall -Wextra -o $@ $<

.PHONY: all
