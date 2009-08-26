sym: sym.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -g -DDD_DEBUG sym.c ../../util/libddutil-dbg.a -o sym

../../util/libddutil-dbg.a:
	make -C ../../util clean
	make -C ../../util libddutil-dbg.a

