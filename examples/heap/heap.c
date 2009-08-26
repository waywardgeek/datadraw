/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "prdatabase.h"

prRoot prTheRoot;



/*--------------------------------------------------------------------------------------------------
  Compare two Nodes.  Return negative is first node gets popped off first.
  Return positve if second node gets popped off first.
--------------------------------------------------------------------------------------------------*/
int prRootCompareNode(
    prNode a,
    prNode b)
{
  utSym aSym = prNodeGetSym(a);
  utSym bSym = prNodeGetSym(b);
  char *aString = utSymGetName(aSym);
  char *bString = utSymGetName(bSym);

  return strcmp(aString, bString);
}

/*--------------------------------------------------------------------------------------------------
  Create a new node.
--------------------------------------------------------------------------------------------------*/
prNode prNodeCreate(
    utSym name)
{
    prNode node = prNodeAlloc();

    prNodeSetSym(node, name);
    return node;
}

/*--------------------------------------------------------------------------------------------------
  Interpret commands and modify the graph database.
--------------------------------------------------------------------------------------------------*/
static void readWriteTree(
    char *filename)
{
    char buffer[1024];
    FILE *file = fopen(filename, "r");
    prNode node;

    if(file == NULL) {
        printf("Unable to open file %s\n", filename);
        exit(1);
    }
    while(fscanf(file, "%s", buffer) >= 0) {
        node = prNodeCreate(utSymCreate(buffer));
        prRootAppendNode(prTheRoot, node);
    }
    fclose(file);
    utDo {
        node = prRootPopNode(prTheRoot);
    } utWhile(node != prNodeNull) {
        printf("%s\n", utSymGetName(prNodeGetSym(node)));
    } utRepeat;
}

/*--------------------------------------------------------------------------------------------------
  This is the actual main routine.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char **argv)
{
    utStart();
    utInitLogFile("graph.log");
    prDatabaseStart();
    prTheRoot = prRootAlloc();
    readWriteTree(argv[1]);
    prDatabaseStop();
    utStop(false);
    return 0;
}
