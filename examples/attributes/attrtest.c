/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "htdatabase.h"

htGraph htTheGraph;
utSym htNameSym, htXSym, htYSym;

/*--------------------------------------------------------------------------------------------------
  Create a new node.
--------------------------------------------------------------------------------------------------*/
htNode htNodeCreate(void)
{
    htNode node = htNodeAlloc();

    htGraphAppendNode(htTheGraph, node);
    return node;
}

/*--------------------------------------------------------------------------------------------------
  Read in the nodes from the text file.
--------------------------------------------------------------------------------------------------*/
static void readNodes(
    char *filename)
{
    char buffer[1024];
    FILE *file = fopen(filename, "r");
    htNode node;
    int x, y;

    if(file == NULL) {
        printf("Unable to open file %s\n", filename);
        exit(1);
    }
    while(fscanf(file, "%s %d %d", buffer, &x, &y) == 3) {
        node = htNodeCreate();
        htNodeSetStringAttribute(node, htNameSym, buffer);
        htNodeSetInt64Attribute(node, htXSym, x);
        htNodeSetInt64Attribute(node, htYSym, y);
    }
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Just write out the nodes.
--------------------------------------------------------------------------------------------------*/
static void writeNodes(void)
{
    htNode node;

    htForeachGraphNode(htTheGraph, node) {
        printf("%s %d %d\n", htNodeGetStringAttribute(node, htNameSym),
            (int32)htNodeGetInt64Attribute(node, htXSym), (int32)htNodeGetInt64Attribute(node, htYSym));
    } htEndGraphNode;
}

/*--------------------------------------------------------------------------------------------------
  This is the actual main routine.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char **argv)
{
    utStart();
    utInitLogFile("hash.log");
    htDatabaseStart();
    htTheGraph = htGraphAlloc();
    htNameSym = utSymCreate("Name");
    htXSym = utSymCreate("X");
    htYSym = utSymCreate("Y");
    readNodes(argv[1]);
    writeNodes();
    htDatabaseStop();
    utStop(false);
    return 0;
}
