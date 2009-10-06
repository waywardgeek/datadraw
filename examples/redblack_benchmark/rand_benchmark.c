#include <stdlib.h>
#include "benchmark.h"

#define NUM_NODES 5000000

static void nodeCreate(
    olRoot root,
    uint32 key)
{
    olNode node = olNodeAlloc();

    olNodeSetKey(node, key);
    olRootInsertNode(root, node);
}

int main(
    int argc,
    char **argv)
{
    olRoot root;
    olNode node;
    uint32 xNode, randKey;

    utStart();
    utInitLogFile("rand_benchmark.log");
    olDatabaseStart();
    root = olRootAlloc();
    if(argc != 1) {
        utExit("Usage: benchmark");
    }
    for(xNode = 0; xNode < NUM_NODES; xNode++) {
        randKey = rand();
        nodeCreate(root, randKey);
        /* utDebug("===================================================================== Create 0x%x %u\n",
            olNode2Index(node), randKey);
        olDumpOrderedList(root);
        olVerifyOrderedList(root); */
    }
    /* olDumpOrderedList(root); */
    for(xNode = 0; xNode < NUM_NODES; xNode++) {
        randKey = rand();
        nodeCreate(root, randKey);
        /* utDebug("===================================================================== Create 0x%x %u\n",
            olNode2Index(node), randKey);
        olDumpOrderedList(root);
        olVerifyOrderedList(root); */
        randKey = rand();
        node = olRootFindPrevNode(root, randKey);
        if(node == olNodeNull) {
            node = olRootGetLastNode(root);
        }
        /* utDebug("===================================================================== Destory 0x%x %u\n",
            olNode2Index(node), randKey); */
        olRootRemoveNode(root, node);
        olNodeFree(node);
        /* olDumpOrderedList(root);
        olVerifyOrderedList(root); */
    }
    olVerifyOrderedList(root);
    olReportStats(root);
    olSafeForeachRootNode(root, node) {
        olRootRemoveNode(root, node);
        olNodeFree(node);
    } olEndSafeRootNode;
    olRootDestroy(root);
    olDatabaseStop();
    utStop(true);
    return 0;
}
