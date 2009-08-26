//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <assert.h>
#include <iostream>

#include <vector>
#include <algorithm>
#include <utility>


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>

const unsigned int graphWidth = 2000;
const unsigned int numTraversals = 100;

using namespace boost;
using namespace std;


template <class VisitorList>
struct edge_categorizer : public dfs_visitor<VisitorList> {
  typedef dfs_visitor<VisitorList> Base;

  edge_categorizer(const VisitorList& v = null_visitor()) : Base(v) { }

  template <class Edge, class Graph>
  void tree_edge(Edge e, Graph& G) {
    Base::tree_edge(e, G);
  }
  template <class Edge, class Graph>
  void back_edge(Edge e, Graph& G) {
    Base::back_edge(e, G);
  }
  template <class Edge, class Graph>
  void forward_or_cross_edge(Edge e, Graph& G) {
    Base::forward_or_cross_edge(e, G);
  }
};
template <class VisitorList>
edge_categorizer<VisitorList>
categorize_edges(const VisitorList& v) {
  return edge_categorizer<VisitorList>(v);
}

static void buildGraphEdges(
    adjacency_list<> mygraph)
{
    for(unsigned int xRow = 0; xRow < graphWidth; xRow++) {
        for(unsigned int xCol = 0; xCol < graphWidth; xCol++) {
            if(xCol > 0) {
		add_edge(xRow, xCol - 1, mygraph);
            }
            if(xRow > 0) {
		add_edge(xRow - 1 , xCol, mygraph);
            }
        }
    }
}

int 
main(int , char* [])
{

  using namespace boost;
  
  typedef adjacency_list<> Graph;
  
  Graph G(graphWidth*graphWidth);

  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef graph_traits<Graph>::vertices_size_type size_type;

  std::vector<size_type> d(num_vertices(G));  
  std::vector<size_type> f(num_vertices(G));
  for(unsigned int xTraversal = 0; xTraversal < numTraversals; xTraversal++) {
    int t = 0;
    depth_first_search(G, visitor(categorize_edges(
                     make_pair(stamp_times(&d[0], t, on_discover_vertex()),
                               stamp_times(&f[0], t, on_finish_vertex())))));
  }

  return 0;
}

