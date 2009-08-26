#include <stdio.h>
#include <stdlib.h>

typedef struct grGraphStruct *grGraph;
typedef struct grNodeStruct *grNode;
typedef struct grEdgeStruct *grEdge;

struct grGraphStruct {
    grNode *nodes;
    unsigned int numNodes;
};

struct grNodeStruct {
    char visited;
    unsigned int color;
    unsigned int level;
    grNode nextGraphNode;
    grEdge firstOutEdge, firstInEdge;
};

struct grEdgeStruct {
    unsigned int weight;
    int riseDelay;
    int fallDelay;
    char enabled;
    grNode fromNode, toNode;
    grEdge nextNodeOutEdge, nextNodeInEdge;
};

#define GRAPH_WIDTH 2000
#define NUM_TRAVERSALS 100

/*-------------------------------------------------------------------------------------------------
  Macros to access 2-D array.
-------------------------------------------------------------------------------------------------*/
static inline grNode grGraphGeti2Node(grGraph graph, unsigned int row, unsigned int col) {
    return graph->nodes[row*GRAPH_WIDTH + col];}
static inline void grGraphSeti2Node(grGraph graph, unsigned int row, unsigned int col, grNode node) {
    graph->nodes[row*GRAPH_WIDTH + col] = node;}

/*-------------------------------------------------------------------------------------------------
  Create a new edge from the first node to the second.
-------------------------------------------------------------------------------------------------*/
static grEdge edgeCreate(
    grNode node1,
    grNode node2)
{
    grEdge edge = (grEdge)malloc(sizeof(struct grEdgeStruct));

    edge->fromNode = node1;
    edge->nextNodeOutEdge = node1->firstOutEdge;
    node1->firstOutEdge = edge;
    edge->toNode = node2;
    edge->nextNodeInEdge = node2->firstOutEdge;
    node2->firstInEdge = edge;
    return edge;
}

/*-------------------------------------------------------------------------------------------------
  Create a new node.
-------------------------------------------------------------------------------------------------*/
static grNode grNodeCreate(
    grGraph graph,
    unsigned int row,
    unsigned int col)
{
    grNode node = (grNode)malloc(sizeof(struct grNodeStruct));

    node->firstOutEdge = NULL;
    node->firstInEdge = NULL;
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
    grGraph graph = (grGraph)malloc(sizeof(struct grGraphStruct));
    grNode node;
    unsigned int xRow, xCol;

    graph->nodes = malloc(sizeof(grNode)*GRAPH_WIDTH*GRAPH_WIDTH);
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
    return graph;
}

/*-------------------------------------------------------------------------------------------------
  Traverse the entire graph reachable through forward edges from this node.  Return the number
  of nodes reached.
-------------------------------------------------------------------------------------------------*/
static unsigned int traverseGraph(
    grNode node)
{
    grNode nextNode;
    grEdge edge;
    unsigned int numNodes = 1;

    node->visited = 1;
    for(edge = node->firstOutEdge; edge != NULL; edge = edge->nextNodeOutEdge) {
        nextNode = edge->toNode;
        if(!nextNode->visited) {
            numNodes += traverseGraph(nextNode);
        }
    }
    return numNodes;
}

/*-------------------------------------------------------------------------------------------------
  Clear visited flags on all nodes of the graph.
-------------------------------------------------------------------------------------------------*/
static unsigned int clearNodeFlags(
    grGraph graph)
{
    grNode node;
    unsigned int xNode;

    for(xNode = 0; xNode < GRAPH_WIDTH*GRAPH_WIDTH; xNode++) {
        node = graph->nodes[xNode];
        node->visited = 0;
    }
}

/*-------------------------------------------------------------------------------------------------
  Main routine.  Build a grid of nodes connected to neighbors, and then traverse them
  NUM_TRAVERSALS times.
-------------------------------------------------------------------------------------------------*/
int main(void)
{
    grGraph graph;
    unsigned int xTraversal;

    graph = createGridGraph();
    for(xTraversal = 0; xTraversal < NUM_TRAVERSALS; xTraversal++) {
        clearNodeFlags(graph);
        if(traverseGraph(grGraphGeti2Node(graph, 0, 0)) != GRAPH_WIDTH*GRAPH_WIDTH) {
            printf("Incorrect graph depth!\n");
            return 1;
        }
    }
    return 0;
}
