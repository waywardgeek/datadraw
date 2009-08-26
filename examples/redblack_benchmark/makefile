all: ordered_benchmark rand_benchmark loop_benchmark multiset_benchmark multisetloop_benchmark

ordered_benchmark: ordered_benchmark.c oldatabase.c oldatabase.h benchmark.h util.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 ordered_benchmark.c oldatabase.c util.c ../../util/libddutil.a -o ordered_benchmark
	#gcc -g -DDD_DEBUG ordered_benchmark.c oldatabase.c util.c ../../util/libddutil-dbg.a -o ordered_benchmark

rand_benchmark: rand_benchmark.c oldatabase.c oldatabase.h benchmark.h util.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 -g rand_benchmark.c oldatabase.c util.c ../../util/libddutil.a -o rand_benchmark
	#gcc -g -DDD_DEBUG rand_benchmark.c oldatabase.c util.c ../../util/libddutil-dbg.a -o rand_benchmark

loop_benchmark: loop_benchmark.c oldatabase.c oldatabase.h benchmark.h util.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 loop_benchmark.c oldatabase.c util.c ../../util/libddutil.a -o loop_benchmark
	#gcc -g -DDD_DEBUG loop_benchmark.c oldatabase.c util.c ../../util/libddutil-dbg.a -o loop_benchmark


multiset_benchmark: multiset_benchmark.cpp
	g++ -O2 -g multiset_benchmark.cpp -o multiset_benchmark

multisetloop_benchmark: multisetloop_benchmark.cpp
	g++ -O2 -g multisetloop_benchmark.cpp -o multisetloop_benchmark

../../util/libddutil.a:
	make -C ../../util clean
	make -C ../../util libddutil.a

oldatabase.c oldatabase.h: Benchmark.dd
	../../src/datadraw Benchmark.dd
