#include "benchmark.h"

#define NUM_NODES 5000000
#define NUM_LOOPS 50

static olNode nodeCreate(
    olRoot root,
    uint32 key)
{
    olNode node = olNodeAlloc();

    olNodeSetKey(node, key);
    olRootInsertNode(root, node);
    return node;
}

int main(
    int argc,
    char **argv)
{
    olRoot root;
    olNode node;
    uint32 xNode, xLoop, keySum = 0, totalKeys = 0;

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
    for(xLoop = 0; xLoop < NUM_LOOPS; xLoop++) {
	olForeachRootNode(root, node) {
	    keySum += olNodeGetKey(node);
	    totalKeys++;
	} olEndRootNode;
    }
    /* olVerifyOrderedList(root);
    olReportStats(root); */
    olSafeForeachRootNode(root, node) {
        olRootRemoveNode(root, node);
        olNodeFree(node);
    } olEndSafeRootNode;
    utLogMessage("Key sum = %u, total keys = %u", keySum, totalKeys);
    olRootDestroy(root);
    olDatabaseStop();
    utStop(true);
    return 0;
}
