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
    uint32 xNode;

    utStart();
    utInitLogFile("ordered_benchmark.log");
    olDatabaseStart();
    root = olRootAlloc();
    if(argc != 1) {
        utExit("Usage: benchmark");
    }
    for(xNode = 0; xNode < NUM_NODES; xNode++) {
        nodeCreate(root, xNode);
    }
    for(xNode = NUM_NODES; xNode < NUM_NODES << 1; xNode++) {
        nodeCreate(root, xNode);
        node = olRootGetFirstNode(root);
        olRootRemoveNode(root, node);
        olNodeFree(node);
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
