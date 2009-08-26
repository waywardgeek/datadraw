/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "prdatabase.h"

prRoot prTheRoot;
char *prLineBuffer;
uint32 prLineSize;

/*--------------------------------------------------------------------------------------------------
  Read a line of text from stdin.
--------------------------------------------------------------------------------------------------*/
static void readLine(void)
{
    uint32 linePosition = 0;
    int c;

    utDo {
        c = getchar();
    } utWhile(c != '\n') {
        if(linePosition == prLineSize) {
            prLineSize <<= 1;
            utResizeArray(prLineBuffer, prLineSize);
        }
        prLineBuffer[linePosition++] = c;
    } utRepeat;
    if(linePosition == prLineSize) {
        prLineSize <<= 1;
        utResizeArray(prLineBuffer, prLineSize);
    }
    prLineBuffer[linePosition] = '\0';
}

/*--------------------------------------------------------------------------------------------------
  Create a new graph.
--------------------------------------------------------------------------------------------------*/
prGraph prGraphCreate(
    utSym name)
{
    prGraph graph = prGraphAlloc();

    prGraphSetSym(graph, name);
    prRootInsertGraph(prTheRoot, graph);
    return graph;
}

/*--------------------------------------------------------------------------------------------------
  Create a new node.
--------------------------------------------------------------------------------------------------*/
prNode prNodeCreate(
    prGraph graph,
    utSym name)
{
    prNode node = prNodeAlloc();

    prNodeSetSym(node, name);
    prGraphAppendNode(graph, node);
    return node;
}

/*--------------------------------------------------------------------------------------------------
  Create a new edge.
--------------------------------------------------------------------------------------------------*/
prEdge prEdgeCreate(
    prNode fromNode,
    prNode toNode)
{
    prEdge edge = prEdgeAlloc();

    prNodeAppendOutEdge(fromNode, edge);
    prNodeAppendInEdge(toNode, edge);
    return edge;
}

/*--------------------------------------------------------------------------------------------------
  Print help.
--------------------------------------------------------------------------------------------------*/
static void printHelp(void)
{
    printf(
        "Commands are:\n"
        "    help - Show this help summary\n"
        "    graph <graph name> - Create or select a graph\n"
        "    destroy_graph - Destroy the current graph\n"
        "    node <node name> <dest nodes>... - Create a node and it's outgoing edges\n"
        "    destroy_node <node name> - Destroy a node\n"
        "    destroy_edge <from node name> <to node name> - destroy an edge\n"
        "    manager - switch to the database manager\n"
        "    reset - Delete everything and start over\n"
        "    list - lists graph name/number pairs\n"
        "    show [graph name] - shows contents of the graph\n"
        "    quit - quit this program\n"
        "    redo - redo the last undone command\n"
        "    undo - undo the last command\n"
        "    compact - compact the database and reset the undo buffer\n");
}

/*--------------------------------------------------------------------------------------------------
  Process a create_graph command.
--------------------------------------------------------------------------------------------------*/
static bool processCreateGraphCommand(void)
{
    prGraph graph;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        printf("Expected graph name\n");
        return false;
    }
    graph = prRootFindGraph(prTheRoot, utSymCreate(token));
    if(graph != prGraphNull) {
        prRootSetCurrentGraph(prTheRoot, graph);
        printf("Selected existing graph %s\n", token);
        return true;
    }
    graph = prGraphCreate(utSymCreate(token));
    prRootSetCurrentGraph(prTheRoot, graph);
    printf("New graph %s\n", token);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Process a destroy_graph command.
--------------------------------------------------------------------------------------------------*/
static bool processDestroyGraphCommand(void)
{
    prGraph graph;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        graph = prRootGetCurrentGraph(prTheRoot);
    } else {
        graph = prRootFindGraph(prTheRoot, utSymCreate(token));
    }
    if(graph == prGraphNull) {
        printf("No graph selected to destroy\n");
        return false;
    }
    printf("Destroyed graph '%s'\n", prGraphGetName(graph));
    prGraphDestroy(graph);
    if(prRootGetCurrentGraph(prTheRoot) == graph) {
        prRootSetCurrentGraph(prTheRoot, prRootGetLastGraph(prTheRoot));
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Process a create_node command.
--------------------------------------------------------------------------------------------------*/
static bool processCreateNodeCommand(void)
{
    prNode node, destNode;
    char *token = strtok(NULL, " \t");

    if(prRootGetCurrentGraph(prTheRoot) == prGraphNull) {
        printf("No current graph selected\n");
        return false;
    }
    if(token == NULL) {
        printf("Expected node name\n");
        return false;
    }
    node = prGraphFindNode(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
    if(node != prNodeNull) {
        printf("Adding edges to existing node '%s'\n", prNodeGetName(node));
    } else {
        node = prNodeCreate(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
        printf("New node %s\n", token);
    }
    utDo {
        token = strtok(NULL, " \t");
    } utWhile(token != NULL) {
        destNode = prGraphFindNode(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
        if(destNode == prNodeNull) {
            destNode = prNodeCreate(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
        }
        prEdgeCreate(node, destNode);
    } utRepeat;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Process a create_node command.
--------------------------------------------------------------------------------------------------*/
static bool processDestroyNodeCommand(void)
{
    prNode node;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        printf("Expected node name\n");
        return false;
    }
    node = prGraphFindNode(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
    if(node == prNodeNull) {
        printf("Node '%s' does not exist\n", token);
        return false;
    }
    prNodeDestroy(node);
    printf("Destroyed node '%s'\n", token);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Find an edge from the frist node to the second.
--------------------------------------------------------------------------------------------------*/
prEdge prFindEdge(
    prNode fromNode,
    prNode toNode)
{
    prEdge edge;

    prForeachNodeOutEdge(fromNode, edge) {
        if(prEdgeGetToNode(edge) == toNode) {
            return edge;
        }
    } prEndNodeOutEdge;
    return prEdgeNull;
}

/*--------------------------------------------------------------------------------------------------
  Process a destroy_edge command.
--------------------------------------------------------------------------------------------------*/
static bool processDestroyEdgeCommand(void)
{
    prNode fromNode, toNode;
    prEdge edge;
    char *token = strtok(NULL, " \t");

    if(prRootGetCurrentGraph(prTheRoot) == prGraphNull) {
        printf("No current graph\n");
        return false;
    }
    if(token == NULL) {
        printf("Expected from node name\n");
        return false;
    }
    fromNode = prGraphFindNode(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
    if(fromNode == prNodeNull) {
        printf("Node '%s' does not exist\n", token);
        return false;
    }
    token = strtok(NULL, " \t");
    if(token == NULL) {
        printf("Expecting to node name\n");
        return false;
    }
    toNode = prGraphFindNode(prRootGetCurrentGraph(prTheRoot), utSymCreate(token));
    if(toNode == prNodeNull) {
        printf("Node '%s' does not exist\n", token);
        return false;
    }
    edge = prFindEdge(fromNode, toNode);
    if(edge == prEdgeNull) {
        printf("Edge does not exist\n");
        return false;
    }
    prEdgeDestroy(edge);
    printf("Destroyed edge %s -> %s\n", prNodeGetName(fromNode), prNodeGetName(toNode));
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Show the graphs in the database.
--------------------------------------------------------------------------------------------------*/
static void showGraphs(void)
{
    prGraph graph;

    prForeachRootGraph(prTheRoot, graph) {
        if(graph == prRootGetCurrentGraph(prTheRoot)) {
            printf("graph %s (current graph)\n", prGraphGetName(graph));
        } else {
            printf("graph %s\n", prGraphGetName(graph));
        }
    } prEndRootGraph;
}

/*--------------------------------------------------------------------------------------------------
  Process a show_graph command.
--------------------------------------------------------------------------------------------------*/
static void showGraphCommand(void)
{
    prGraph graph;
    prNode node;
    prEdge edge;
    char *token = strtok(NULL, " \t");

    if(token == NULL) {
        graph = prRootGetCurrentGraph(prTheRoot);
        if(graph == prGraphNull) {
            printf("No current graph set\n");
            return;
        }
    } else {
        graph = prRootFindGraph(prTheRoot, utSymCreate(token));
        if(graph == prGraphNull) {
            printf("Graph '%s' does not exist\n", token);
            return;
        }
    }
    printf("graph %s\n", prGraphGetName(graph));
    prForeachGraphNode(graph, node) {
        printf("    node %s", prNodeGetName(node));
        prForeachNodeOutEdge(node, edge) {
            printf(" %s", prNodeGetName(prEdgeGetToNode(edge)));
        } prEndNodeOutEdge;
        printf("\n");
    } prEndGraphNode;
}

/*--------------------------------------------------------------------------------------------------
  Process the command in the line buffer.
--------------------------------------------------------------------------------------------------*/
static bool processCommand(
    bool *didSomething)
{
    char *token;

    *didSomething = false;
    token = strtok(prLineBuffer, " \t");
    if(token == NULL) {
        return true;
    }
    if(prLineBuffer[0] == '\0') {
        return true; /* Empty line */
    }
    if(!strcmp(token, "help")) {
        printHelp();
    } else if(!strcmp(token, "graph")) {
        if(processCreateGraphCommand()) {
            *didSomething = true;
        }
    } else if(!strcmp(token, "destroy_graph")) {
        if(processDestroyGraphCommand()) {
            *didSomething = true;
        }
    } else if(!strcmp(token, "node")) {
        if(processCreateNodeCommand()) {
            *didSomething = true;
        }
    } else if(!strcmp(token, "destroy_node")) {
        if(processDestroyNodeCommand()) {
            *didSomething = true;
        }
    } else if(!strcmp(token, "destroy_edge")) {
        if(processDestroyEdgeCommand()) {
            *didSomething = true;
        }
    } else if(!strcmp(token, "reset")) {
        utResetDatabase();
        prTheRoot = prRootAlloc();
    } else if(!strcmp(token, "compact")) {
        utCompactDatabase();
    } else if(!strcmp(token, "list")) {
        showGraphs();
    } else if(!strcmp(token, "manager")) {
        utManager();
    } else if(!strcmp(token, "show")) {
        showGraphCommand();
    } else if(!strcmp(token, "quit")) {
        return false;
    } else if(!strcmp(token, "redo")) {
        if(utRedo(1) == 0) {
            printf("No more commands to redo\n");
        }
    } else if(!strcmp(token, "undo")) {
        if(utUndo(1) == 0) {
            printf("No more commands to undo\n");
        }
    } else {
        printf("Invalid command.  Type 'help' for a list of commands\n");
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Interpret commands and modify the graph database.
--------------------------------------------------------------------------------------------------*/
static void commandInterpreter(void)
{
    bool didSomething;

    printf("For help, enter the 'help' command\n");
    utDo {
        printf("> ");
        readLine();
    } utWhile(processCommand(&didSomething)) {
        if(didSomething) {
            utTransactionComplete(true);
        }
    } utRepeat;
}

/*--------------------------------------------------------------------------------------------------
  This is the actual main routine.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char **argv)
{
    if(argc != 1) {
        printf("Usage: %s\n"
               "    This will launch a comand shell for manipulating graphs.\n",
            argv[0]);
        exit(1);
    }
    utStart();
    utInitLogFile("graph.log");
    prDatabaseStart();
    prTheRoot = prRootAlloc();
    utStartPersistence("graph_database", false, true);
    prLineSize = 42;
    prLineBuffer = utNewA(char, prLineSize);
    commandInterpreter();
    utStopPersistence();
    prDatabaseStop();
    utFree(prLineBuffer);
    utStop(false);
    return 0;
}
