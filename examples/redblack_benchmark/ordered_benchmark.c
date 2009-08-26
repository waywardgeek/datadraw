#include "benchmark.h"

#define NUM_NODES 5000000

static olNode nodeCreate(
    olRoot root,
    uint32 key)
{
    olNode node = olNodeAllocRaw();

    olNodeSetKey(node, key);
    olNodeSetRoot(node, root);
    olRootInsertNode(root, node);
    return node;
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
        node = nodeCreate(root, xNode);
        /* utDebug("========================================= created 0x%x\n", olNode2Index(node));
        olDumpOrderedList(root);
        olVerifyOrderedList(root); */
    }
    for(xNode = NUM_NODES; xNode < NUM_NODES << 1; xNode++) {
        node = nodeCreate(root, xNode);
        /* utDebug("========================================= created 0x%x\n", olNode2Index(node));
        olDumpOrderedList(root);
        olVerifyOrderedList(root); */
        node = olRootGetFirstNode(root);
        olRootRemoveNode(root, node);
        /* utDebug("========================================= Destroyed 0x%x\n", olNode2Index(node)); */
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
