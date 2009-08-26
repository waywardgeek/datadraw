all: c_graph cpp_graph rawc_graph

c_graph: graph.c grdatabase.c grdatabase.h ../../util/ddutil.h ../../util/libddutil-dbg.a
	gcc -O2 graph.c grdatabase.c ../../util/libddutil.a -o c_graph
	#gcc -g -DDD_DEBUG graph.c grdatabase.c ../../util/libddutil-dbg.a -o c_graph

cpp_graph: graph.cpp
	g++ -O2 graph.cpp -o cpp_graph
	#g++ -g graph.cpp -o cpp_graph

rawc_graph: rawc_graph.c
	gcc -O2 rawc_graph.c -o rawc_graph
	#gcc -g rawc_graph.c -o rawc_graph

../../util/libddutil.a:
	make -C ../../util clean
	make -C ../../util libddutil.a

grdatabase.c grdatabase.h: Graph.dd
	../../src/datadraw Graph.dd
