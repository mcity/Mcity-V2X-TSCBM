all : library main

library : tscbm.c
	cc -fPIC -shared -o libtscbm.so tscbm.c
main : tscbm.c
	cc tscbm.c

clean :
	rm -f a.out libtscbm.so