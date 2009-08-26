/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "grdatabase.h"

grRoot grTheRoot;

static char *grLineBuffer;
static uint32 grLineSize;
static FILE *grCommandFile;

/*--------------------------------------------------------------------------------------------------
  Find the first edge on the node.
--------------------------------------------------------------------------------------------------*/
grEdge grNodeGetFirstEdge(
    grNode node)
{
    grEdge edge = grNodeGetFirstOutEdge(node);

    if(edge != grEdgeNull) {
        return edge;
    }
    return grNodeGetFirstInEdge(node);
}

/*--------------------------------------------------------------------------------------------------
  Find the next edge on the node.
--------------------------------------------------------------------------------------------------*/
grEdge grEdgeGetNextNodeEdge(
    grNode node,
    grEdge edge)
{
    grEdge nextEdge;

    if(grEdgeGetFromNode(edge) == node) {
        nextEdge = grEdgeGetNextNodeOutEdge(edge);
        if(nextEdge != grEdgeNull) {
            return nextEdge;
        }
        return grNodeGetFirstInEdge(node);
    }
    return grEdgeGetNextNodeInEdge(edge);
}

/*--------------------------------------------------------------------------------------------------
  Read a line of text from stdin.
--------------------------------------------------------------------------------------------------*/
static bool readLine(void)
{
    uint32 linePosition = 0;
    int c = getc(grCommandFile);

    if(c == EOF) {
        return false;
    }
    while(c != '\n' && c != EOF) {
        if(linePosition == grLineSize) {
            grLineSize <<= 1;
            utResizeArray(grLineBuffer, grLineSize);
        }
        grLineBuffer[linePosition++] = c;
        c = getc(grCommandFile);
    }
    if(linePosition == grLineSize) {
        grLineSize <<= 1;
        utResizeArray(grLineBuffer, grLineSize);
    }
    grLineBuffer[linePosition] = '\0';
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Create a new graph.
--------------------------------------------------------------------------------------------------*/
grGraph grGraphCreate(
    utSym name)
{
    grGraph graph = grGraphAlloc();

    grGraphSetSym(graph, name);
    grRootAppendGraph(grTheRoot, graph);
    return graph;
}

/*--------------------------------------------------------------------------------------------------
  Create a new node.
--------------------------------------------------------------------------------------------------*/
grNode grNodeCreate(
    grGraph graph,
    utSym name)
{
    grNode node = grNodeAlloc();

    grNodeSetSym(node, name);
    grGraphAppendNode(graph, node);
    return node;
}

/*--------------------------------------------------------------------------------------------------
  Create a new edge.
--------------------------------------------------------------------------------------------------*/
grEdge grEdgeCreate(
    grNode fromNode,
    grNode toNode)
{
    grEdge edge = grEdgeAlloc();

    grNodeAppendOutEdge(fromNode, edge);
    grNodeAppendInEdge(toNode, edge);
    return edge;
}

/*--------------------------------------------------------------------------------------------------
  Process a create_graph command.
--------------------------------------------------------------------------------------------------*/
static bool processCreateGraphCommand(void)
{
    grGraph graph;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        printf("Expected graph name\n");
        return false;
    }
    graph = grRootFindGraph(grTheRoot, utSymCreate(token));
    if(graph != grGraphNull) {
        grRootSetCurrentGraph(grTheRoot, graph);
        printf("Selected existing graph %s\n", token);
        return true;
    }
    graph = grGraphCreate(utSymCreate(token));
    grRootSetCurrentGraph(grTheRoot, graph);
    printf("New graph %s\n", token);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Process a create_node command.
--------------------------------------------------------------------------------------------------*/
static bool processCreateNodeCommand(void)
{
    grNode node, destNode;
    char *token = strtok(NULL, " \t");

    if(grRootGetCurrentGraph(grTheRoot) == grGraphNull) {
        printf("No current graph selected\n");
        return false;
    }
    if(token == NULL) {
        printf("Expected node name\n");
        return false;
    }
    node = grGraphFindNode(grRootGetCurrentGraph(grTheRoot), utSymCreate(token));
    if(node != grNodeNull) {
        printf("Adding edges to existing node '%s'\n", grNodeGetName(node));
    } else {
        node = grNodeCreate(grRootGetCurrentGraph(grTheRoot), utSymCreate(token));
        printf("New node %s\n", token);
    }
    utDo {
        token = strtok(NULL, " \t");
    } utWhile(token != NULL) {
        destNode = grGraphFindNode(grRootGetCurrentGraph(grTheRoot), utSymCreate(token));
        if(destNode == grNodeNull) {
            destNode = grNodeCreate(grRootGetCurrentGraph(grTheRoot), utSymCreate(token));
        }
        grEdgeCreate(node, destNode);
    } utRepeat;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Show the graphs in the database.
--------------------------------------------------------------------------------------------------*/
static void showGraphs(void)
{
    grGraph graph;

    grForeachRootGraph(grTheRoot, graph) {
        if(graph == grRootGetCurrentGraph(grTheRoot)) {
            printf("graph %s (current graph)\n", grGraphGetName(graph));
        } else {
            printf("graph %s\n", grGraphGetName(graph));
        }
    } grEndRootGraph;
}

/*--------------------------------------------------------------------------------------------------
  Process a show_graph command.
--------------------------------------------------------------------------------------------------*/
static void showGraphCommand(void)
{
    grGraph graph;
    grNode node;
    grEdge edge;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        graph = grRootGetCurrentGraph(grTheRoot);
        if(graph == grGraphNull) {
            printf("No current graph set\n");
            return;
        }
    } else {
        graph = grRootFindGraph(grTheRoot, utSymCreate(token));
        if(graph == grGraphNull) {
            printf("Graph '%s' does not exist\n", token);
            return;
        }
    }
    printf("graph %s\n", grGraphGetName(graph));
    grForeachGraphNode(graph, node) {
        printf("    node %s", grNodeGetName(node));
        grForeachNodeOutEdge(node, edge) {
            printf(" %s", grNodeGetName(grEdgeGetToNode(edge)));
        } grEndNodeOutEdge;
        printf("\n");
    } grEndGraphNode;
}

/*--------------------------------------------------------------------------------------------------
  Process the command in the line buffer.
--------------------------------------------------------------------------------------------------*/
static void processCommand(void)
{
    char *token;

    token = strtok(grLineBuffer, " \t");
    if(token == NULL) {
        return;
    }
    if(grLineBuffer[0] == '\0') {
        return; /* Empty line */
    }
    if(!strcmp(token, "graph")) {
        processCreateGraphCommand();
    } else if(!strcmp(token, "node")) {
        processCreateNodeCommand();
    } else if(!strcmp(token, "list")) {
        showGraphs();
    } else if(!strcmp(token, "show")) {
        showGraphCommand();
    } else {
        printf("Invalid command.  Type 'help' for a list of commands\n");
    }
}

/*--------------------------------------------------------------------------------------------------
  Interpret commands and modify the graph database.
--------------------------------------------------------------------------------------------------*/
void grCommandInterpreter(
    char *commandFile)
{
    grCommandFile = fopen(commandFile, "r");

    if(grCommandFile == NULL) {
        utExit("Unable to open file %s", commandFile);
    }
    while(readLine()) {
        processCommand();
    }
    fclose(grCommandFile);
}

/*--------------------------------------------------------------------------------------------------
  Initialize the graph database.
--------------------------------------------------------------------------------------------------*/
void grStart(void)
{
    grLineSize = 42;
    grLineBuffer = utNewA(char, grLineSize);
    grDatabaseStart();
    grTheRoot = grRootAlloc();
}

/*--------------------------------------------------------------------------------------------------
  Free the graph database.
--------------------------------------------------------------------------------------------------*/
void grStop(void)
{
    grDatabaseStop();
    utFree(grLineBuffer);
}
