#include "benchmark.h"

#define NUM_NODES 5000000

static void nodeCreate(
    olRoot root,
    uint32 key)
{
    olNode node = olNodeAllocRaw();

    olNodeSetKey(node, key);
    olNodeSetRoot(node, root);
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
        randKey = utRand();
        nodeCreate(root, randKey);
    }
    /* olDumpOrderedList(root); */
    for(xNode = 0; xNode < NUM_NODES; xNode++) {
        randKey = utRand();
        nodeCreate(root, randKey);
        /* olVerifyOrderedList(root);
        utDebug("===================================================================== Create 0x%x\n",
            olNode2Index(node));
        olDumpOrderedList(root); */
        randKey = utRand();
        node = olRootFindPrevNode(root, randKey);
        if(node == olNodeNull) {
            node = olRootGetLastNode(root);
        }
        /* utDebug("===================================================================== Destory 0x%x\n",
            olNode2Index(node)); */
        olRootRemoveNode(root, node);
        olNodeFree(node);
        /* olVerifyOrderedList(root);
        olDumpOrderedList(root); */
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
