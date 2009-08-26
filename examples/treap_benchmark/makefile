all: ordered_benchmark rand_benchmark

ordered_benchmark: ordered_benchmark.c oltreap.c oltreap.h benchmark.h util.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 ordered_benchmark.c oltreap.c util.c ../../util/libddutil.a -o ordered_benchmark

rand_benchmark: rand_benchmark.c oltreap.c oltreap.h benchmark.h util.c ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 rand_benchmark.c oltreap.c util.c ../../util/libddutil.a -o rand_benchmark
	#gcc -g rand_benchmark.c oltreap.c util.c ../../util/libddutil-dbg.a -o rand_benchmark
	#gcc -g -pg rand_benchmark.c oltreap.c util.c ../../util/libddutil-dbg.a -o rand_benchmark

../../util/libddutil.a:
	make -C ../../util clean
	make -C ../../util libddutil.a
