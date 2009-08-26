#include "oldatabase.h"

olNode olNodeCreate(
    olRoot root,
    char *name)
{
    olNode node = olNodeAlloc();

    olNodeSetName(node, name, strlen(name) + 1);
    olNodeSetRoot(node, root);
    olRootInsertNode(root, node);
    return node;
}

int olRootCompareNode(
    olNode a,
    olNode b)
{
    uint32 i, m = utMin(olNodeGetNumName(a), olNodeGetNumName(b));
    char *nameA = olNodeGetName(a);
    char *nameB = olNodeGetName(b);

    for(i=0; i < m; i++) {
        if(nameB[i] < nameA[i]) {
            return -1;
        } else if(nameB[i] > nameA[i]) {
            return +1;
        }
    }
    if(olNodeGetNumName(b) < olNodeGetNumName(a)) {
        return -1;
    } else if(olNodeGetNumName(b) > olNodeGetNumName(a)) {
        return +1;
    }
    return 0;
}

static void verifyTree(
    olNode node)
{
    olNode parent = olNodeGetParentRootNode(node);
    olNode left = olNodeGetLeftRootNode(node);
    olNode right = olNodeGetRightRootNode(node);

    utAssert(parent == olNodeNull || olNodeGetLeftRootNode(parent) == node || olNodeGetRightRootNode(parent) == node);
    if(left != olNodeNull) {
        utAssert(!olNodeIsRedRootNode(node) || !olNodeIsRedRootNode(left));
        verifyTree(left);
    }
    if(right != olNodeNull) {
        utAssert(!olNodeIsRedRootNode(node) || !olNodeIsRedRootNode(right));
        verifyTree(right);
    }
}

static void verifyOrderedList(
    olRoot root)
{
    olNode node;
    olNode rootNode = olRootGetRootNode(root);
    olNode prevNode = olNodeNull;

    olForeachRootNode(root, node) {
        utAssert(prevNode == olNodeNull || olRootCompareNode(prevNode, node) >= 0);
    } olEndRootNode;
    if(rootNode != olNodeNull) {
        verifyTree(rootNode);
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

static void dumpTree(
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
    dumpTree(left, depth + 1);
    indent(depth);
    utDebug("%s 0x%x %s\n", olNodeGetName(node), olNode2Index(node), olNodeIsRedRootNode(node)? "red" : "black");
    dumpTree(right, depth + 1);
}

static uint32 countRootNodes(
    olRoot root)
{
    olNode node;
    uint32 numNodes = 0;

    olForeachRootNode(root, node) {
        numNodes++;
    } olEndRootNode;
    return numNodes;
}

static olNode pickRandomNode(
    olRoot root)
{
    olNode node = olRootGetFirstNode(root);
    uint32 numNodes = countRootNodes(root);
    uint32 randIndex;
    uint32 xNode;

    if(node == olNodeNull) {
        return olNodeNull;
    }
    randIndex = utRandN(numNodes);
    for(xNode = 0; xNode < randIndex; xNode++) {
        node = olNodeGetNextRootNode(node);
    }
    return node;
}

int main(
    int argc,
    char **argv)
{
    olRoot root;
    olNode node;
    FILE *f;
    char buffer[1024];

    utStart();
    utInitLogFile("ordered.log");
    olDatabaseStart();
    root = olRootAlloc();
    if(argc != 2) {
        utExit("Usage: ordered file");
    }
    f = fopen(argv[1], "r");
    if(f == NULL) {
        utExit("Unable to open %s", argv[1]);
    }
    while(fscanf(f, "%s", buffer) >= 0) {
        olNodeCreate(root, buffer);
        utDebug("====================================================================\n");
        dumpTree(olRootGetRootNode(root), 0);
        verifyOrderedList(root);
    }
    fclose(f);
    olForeachRootNode(root, node) {
        utLogMessage("%s", olNodeGetName(node));
    } olEndRootNode;
    utDo {
        node = pickRandomNode(root);
    } utWhile(node != olNodeNull) {
        utDebug("====================================================================\n");
        utDebug("Deleting node %s\n", olNodeGetName(node));
        olRootRemoveNode(root, node);
        dumpTree(olRootGetRootNode(root), 0);
        verifyOrderedList(root);
    } utRepeat;
    olRootDestroy(root);
    olDatabaseStop();
    utStop(false);
    return 0;
}
