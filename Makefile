all: reg nfa
	./test.sh
clean: 
	-rm -rf reg nfa

distclean:
	-rm -rf reg nfa G*

reg: reg.c
	gcc $^ -o $@ -std=c99 -g
nfa: nfa.c
	gcc $^ -o $@ -std=c99 -g
