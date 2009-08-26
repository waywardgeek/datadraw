/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "htdatabase.h"

htGraph htTheGraph;

/*--------------------------------------------------------------------------------------------------
  Create a new node.
--------------------------------------------------------------------------------------------------*/
htNode htNodeCreate(
    char *name,
    int32 x,
    int32 y)
{
    htNode node = htGraphFindNode(htTheGraph, x, y);
    uint32 length = strlen(name) + 1;

    if(length > NAME_LENGTH) {
	utExit("Name %s is too long", name);
    }
    if(node != htNodeNull) {
        utExit("Node %s is physically on top of node %s", name, htNodeGetName(node));
    }
    node = htNodeAlloc();
    htNodeSetName(node, name, length);
    htNodeSetX(node, x);
    htNodeSetY(node, y);
    htGraphInsertNode(htTheGraph, node);
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
        node = htNodeCreate(buffer, x, y);
    }
    fclose(file);
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
    readNodes(argv[1]);
    htDatabaseStop();
    utStop(false);
    return 0;
}
