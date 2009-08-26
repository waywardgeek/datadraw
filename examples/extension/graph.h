#include "grdatabase.h"

void grCommandInterpreter(char *commandFile);
void grStart(void);
void grStop(void);

/* Constructors */
grGraph grGraphCreate(utSym name);
grNode grNodeCreate(grGraph graph, utSym name);
grEdge grEdgeCreate(grNode fromNode, grNode toNode);

/* Iterator to traverse both in and out edges */
grEdge grNodeGetFirstEdge(grNode node);
grEdge grEdgeGetNextNodeEdge(grNode node, grEdge edge);
#define grForeachNodeEdge(node, edge) \
    for(edge = grNodeGetFirstEdge(node); edge != grEdgeNull; edge = grEdgeGetNextNodeEdge(node, edge))
#define grEndNodeEdge
#define grEdgeFindOtherNode(edge, node) (grEdgeGetFromNode(edge) == (node)? grEdgeGetToNode(edge) : \
    grEdgeGetFromNode(edge))

extern grRoot grTheRoot;
