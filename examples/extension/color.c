/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "codatabase.h"

#define NUM_RAND_NODES 40000
#define NUM_RAND_EDGES 100000

/*--------------------------------------------------------------------------------------------------
  Just see if a color has been used by a neighbor.
--------------------------------------------------------------------------------------------------*/
static void createLargeRandomGraph(void)
{
    grGraph graph = grGraphCreate(utSymCreate("RandGraph"));
    grNode node, fromNode, toNode;
    grNodeArray nodes = grNodeArrayAlloc();
    int32 xNode, xEdge, xRandFrom, xRandTo;

    srand(42);
    for(xNode = 0; xNode < NUM_RAND_NODES; xNode++) {
        node = grNodeCreate(graph, utSymCreateFormatted("N%u", xNode));
        grNodeArrayAppendNode(nodes, node);
    }
    for(xEdge = 0; xEdge < NUM_RAND_EDGES; xEdge++) {
        xRandFrom = rand() % NUM_RAND_NODES;
        xRandTo = rand() % NUM_RAND_NODES;
        fromNode = grNodeArrayGetiNode(nodes, xRandFrom);
        toNode = grNodeArrayGetiNode(nodes, xRandTo);
        grEdgeCreate(fromNode, toNode);
    }
    grNodeArrayFree(nodes);
}

/*--------------------------------------------------------------------------------------------------
  Just see if a color has been used by a neighbor.
--------------------------------------------------------------------------------------------------*/
static bool colorAlreadyUsed(
    grNode node,
    uint32 color)
{
    grNode otherNode;
    grEdge edge;

    grForeachNodeEdge(node, edge) {
        otherNode = grEdgeFindOtherNode(edge, node);
        if(coNodeColored(otherNode) && coNodeGetColor(otherNode) == color) {
            return true;
        }
    } grEndNodeEdge;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Just find a color that has not yet been used to color adjacent nodes.  It's a dumb N^2 loop.
--------------------------------------------------------------------------------------------------*/
static uint32 findUnusedColor(
    grNode node)
{
    grGraph graph = grNodeGetGraph(node);
    uint32 color;

    for(color = 0; color < coGraphGetNumColors(graph); color++) {
        if(!colorAlreadyUsed(node, color)) {
            return color;
        }
    }
    return UINT32_MAX;
}

/*--------------------------------------------------------------------------------------------------
  This is a lame attempt to color the graph with a stupid greedy algorithm: just pick an unused
  color, and if none exist, color a node 0.
--------------------------------------------------------------------------------------------------*/
static void colorGraph(
    grGraph graph)
{
    grNode node;
    uint32 unusedColor;

    grForeachGraphNode(graph, node) {
        unusedColor = findUnusedColor(node);
        if(unusedColor == UINT32_MAX) {
            unusedColor = 0;
            printf("Node %s has color conflict\n", grNodeGetName(node));
        }
        coNodeSetColor(node, unusedColor);
        coNodeSetColored(node, true);
    } grEndGraphNode;
}

/*--------------------------------------------------------------------------------------------------
  Write out a colored graph.
--------------------------------------------------------------------------------------------------*/
static void writeGraph(
    grGraph graph)
{
    grNode node;

    grForeachGraphNode(graph, node) {
        printf("Node %s color %u\n", grNodeGetName(node), coNodeGetColor(node));
    } grEndGraphNode;
}

/*--------------------------------------------------------------------------------------------------
  This is the actual main routine.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char **argv)
{
    grGraph graph;
    uint32 numColors;
    bool runManager = false;
    uint32 colorsArg = 1;
    uint32 fileArg = 2;

    if(argc < 3 || (argc == 4 && strcmp(argv[1], "-m")) || argc > 4) {
        printf("Usage: %s [-m] <colors> command.txt\n"
               "    -m - this option starts the database manager after coloring.\n"
               "    This will graph commands to build a graph, and then color it.\n",
            argv[0]);
        exit(1);
    }
    if(argc == 4) {
        runManager = true;
        colorsArg = 2;
        fileArg = 3;
    }
    numColors = atoi(argv[colorsArg]);
    utStart();
    grStart();
    createLargeRandomGraph(); /* Just so we have a reason to use a sparse extension */
    grCommandInterpreter(argv[fileArg]); /* Read in a small graph from command line */
    graph = grRootGetLastGraph(grTheRoot); /* Last graph will be the last one created */
    if(graph == grGraphNull) {
        utExit("No graph to color!");
    }
    coDatabaseStart(); /* This extends all the graph and node objects with extra fields */
    coGraphSetNumColors(graph, numColors);
    colorGraph(graph);
    writeGraph(graph);
    if(runManager) {
        utManager();
    }
    coDatabaseStop();
    grStop();
    utStop(true);
    return 0;
}
