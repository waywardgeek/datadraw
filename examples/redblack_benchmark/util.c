#include "benchmark.h"

uint32 olVerifyTree(
    olNode node)
{
    olNode parent = olNodeGetParentRootNode(node);
    olNode left = olNodeGetLeftRootNode(node);
    olNode right = olNodeGetRightRootNode(node);
    uint32 leftDepth = 0, rightDepth = 0;

    utAssert(parent == olNodeNull || olNodeGetLeftRootNode(parent) == node || olNodeGetRightRootNode(parent) == node);
    utAssert(parent != olNodeNull || !olNodeIsRedRootNode(node));
    if(left != olNodeNull) {
        utAssert(!olNodeIsRedRootNode(node) || !olNodeIsRedRootNode(left));
        utAssert(olNodeGetKey(node) >= olNodeGetKey(left));
        leftDepth = olVerifyTree(left);
    }
    if(right != olNodeNull) {
        utAssert(!olNodeIsRedRootNode(node) || !olNodeIsRedRootNode(right));
        utAssert(olNodeGetKey(node) <= olNodeGetKey(right));
        rightDepth = olVerifyTree(right);
    }
    utAssert(leftDepth == rightDepth);
    if(olNodeIsRedRootNode(node)) {
        return leftDepth;
    }
    return leftDepth + 1;
}

void olVerifyOrderedList(
    olRoot root)
{
    olNode node;
    olNode rootNode = olRootGetRootNode(root);
    olNode prevNode = olNodeNull;

    olForeachRootNode(root, node) {
        utAssert(prevNode == olNodeNull || olRootCompareNode(prevNode, node) >= 0);
    } olEndRootNode;
    if(rootNode != olNodeNull) {
        utAssert(olNodeGetParentRootNode(rootNode) == olNodeNull);
        olVerifyTree(rootNode);
    }
}

static indent(
    uint32 depth)
{
    int32 xChar;

    for(xChar = 0; xChar < depth; xChar++) {
        utDebug("\t");
    }
}

void olDumpTree(
    olNode node,
    uint32 depth)
{
    olNode left, right;

    if(node == olNodeNull) {
        indent(depth);
        utDebug("NULL\n");
        return;
    }
    left = olNodeGetLeftRootNode(node);
    right = olNodeGetRightRootNode(node);
    olDumpTree(left, depth + 1);
    indent(depth);
    utDebug("0x%x %u %s\n", olNode2Index(node), olNodeGetKey(node),
        olNodeIsRedRootNode(node)? "red" : "black");
    olDumpTree(right, depth + 1);
}

void olDumpOrderedList(
    olRoot root)
{
    olNode rootNode = olRootGetRootNode(root);

    olDumpTree(rootNode, 0);
}

static uint32 countStats(
    olNode node,
    uint32 depth,
    uint64 *totalPathDepth)
{
    olNode left = olNodeGetLeftRootNode(node);
    olNode right = olNodeGetRightRootNode(node);
    uint32 numNodes = 1;

    *totalPathDepth += depth;
    if(left != olNodeNull) {
        numNodes += countStats(left, depth + 1, totalPathDepth);
    }
    if(right != olNodeNull) {
        numNodes += countStats(right, depth + 1, totalPathDepth);
    }
    return numNodes;
}

void olReportStats(
    olRoot root)
{
    olNode rootNode = olRootGetRootNode(root);
    uint64 totalPathDepth = 0;
    uint32 numNodes;

    if(rootNode == olNodeNull) {
        utLogMessage("This tree is empty");
        return;
    }
    numNodes = countStats(olRootGetRootNode(root), 1, &totalPathDepth);
    utLogMessage("There are %u nodes with average depth %0.1f", numNodes, ((double)totalPathDepth/numNodes));
}
