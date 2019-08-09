all : library main

library : battelle.c
	cc -fPIC -shared -o libbattelle.so battelle.c
main : battelle.c
	cc battelle.c

clean :
	rm -f a.out libbattelle.so