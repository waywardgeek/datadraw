#include <stdio.h>
#include "grdatabase.h"

#define GRAPH_WIDTH 2000
#define NUM_TRAVERSALS 100

/*-------------------------------------------------------------------------------------------------
  Macros to access 2-D array.
-------------------------------------------------------------------------------------------------*/
static inline grNode grGraphGeti2Node(grGraph graph, uint32 row, uint32 col) {
    return grGraphGetiNode(graph, row*GRAPH_WIDTH + col);}
static inline void grGraphSeti2Node(grGraph graph, uint32 row, uint32 col, grNode node) {
    grGraphSetiNode(graph, row*GRAPH_WIDTH + col, node);}

/*-------------------------------------------------------------------------------------------------
  Create a new edge from the first node to the second.
-------------------------------------------------------------------------------------------------*/
static grEdge edgeCreate(
    grNode node1,
    grNode node2)
{
    grEdge edge = grEdgeAlloc();

    grNodeInsertOutEdge(node1, edge);
    grNodeInsertInEdge(node2, edge);
    return edge;
}

/*-------------------------------------------------------------------------------------------------
  Create a new node.
-------------------------------------------------------------------------------------------------*/
static grNode grNodeCreate(
    grGraph graph,
    uint32 row,
    uint32 col)
{
    grNode node = grNodeAlloc();

    grGraphSeti2Node(graph, row, col, node);
    return node;
}

/*-------------------------------------------------------------------------------------------------
  Create a mesh shapped graph which is GRAPH_WIDTH wide and GRAPH_WIDTH tall.  The first node of
  the graph will be a corner with outgoing edges to neighbors, and those neighbors will point
  outward from the first node.
-------------------------------------------------------------------------------------------------*/
static grGraph createGridGraph(void)
{
    grGraph graph = grGraphAlloc();
    grNode node;
    uint32 xRow, xCol;

    grGraphAllocNodes(graph, GRAPH_WIDTH*GRAPH_WIDTH);
    for(xRow = 0; xRow < GRAPH_WIDTH; xRow++) {
        for(xCol = 0; xCol < GRAPH_WIDTH; xCol++) {
            node = grNodeCreate(graph, xRow, xCol);
            if(xCol > 0) {
                edgeCreate(grGraphGeti2Node(graph, xRow, xCol - 1), node);
            }
            if(xRow > 0) {
                edgeCreate(grGraphGeti2Node(graph, xRow - 1, xCol), node);
            }
        }
    }
    grGraphSetUsedNode(graph, GRAPH_WIDTH*GRAPH_WIDTH);
    return graph;
}

/*-------------------------------------------------------------------------------------------------
  Traverse the entire graph reachable through forward edges from this node.  Return the number
  of nodes reached.
-------------------------------------------------------------------------------------------------*/
static uint32 traverseGraph(
    grNode node)
{
    grNode nextNode;
    grEdge edge;
    uint32 numNodes = 1;

    grNodeSetVisited(node, true);
    grForeachNodeOutEdge(node, edge) {
        nextNode = grEdgeGetToNode(edge);
        if(!grNodeVisited(nextNode)) {
            numNodes += traverseGraph(nextNode);
        }
    } grEndNodeOutEdge;
    return numNodes;
}

/*-------------------------------------------------------------------------------------------------
  Clear visited flags on all nodes of the graph.
-------------------------------------------------------------------------------------------------*/
static uint32 clearNodeFlags(
    grGraph graph)
{
    grNode node;

    grForeachGraphNode(graph, node) {
        grNodeSetVisited(node, false);
    } grEndGraphNode;
}

/*-------------------------------------------------------------------------------------------------
  Main routine.  Build a grid of nodes connected to neighbors, and then traverse them
  NUM_TRAVERSALS times.
-------------------------------------------------------------------------------------------------*/
int main(void)
{
    grGraph graph;
    uint32 xTraversal;

    utStart();
    grDatabaseStart();
    graph = createGridGraph();
    for(xTraversal = 0; xTraversal < NUM_TRAVERSALS; xTraversal++) {
        clearNodeFlags(graph);
        utAssert(traverseGraph(grGraphGeti2Node(graph, 0, 0)) == GRAPH_WIDTH*GRAPH_WIDTH);
    }
    grDatabaseStop();
    utStop(true);
    return 0;
}
