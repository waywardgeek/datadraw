ordered: ordered.c oldatabase.c oldatabase.h ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -g -DDD_DEBUG -I../../util ordered.c oldatabase.c ../../util/libddutil-dbg.a -o ordered

../../util/libddutil-dbg.a:
	make -C ../../util clean
	make -C ../../util libddutil-dbg.a

oldatabase.c oldatabase.h: OrderedList.dd
	../../src/datadraw OrderedList.dd

oldatabase.c oldatabase.h:
